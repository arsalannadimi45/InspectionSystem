// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/InspectSubsystem.h"

#include "Actions/InspectAction.h"
#include "Core/InspectDataAsset.h"
#include "Interface/Inspectable.h"
#include "UI/InspectWidget.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Blueprint/UserWidget.h"
#include "Components/InspectableComponent.h"
#include "Components/InspectorComponent.h"
#include "Core/InspectSession.h"
#include "Core/InspectSettings.h"
#include "GameFramework/PlayerController.h"


// Subsystem lifecycle

void UInspectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("[InspectSubsystem] Initialized."));
}

void UInspectSubsystem::Deinitialize()
{
	// Ensure clean teardown even if level changes mid-inspect.
	EndInspect();
	Super::Deinitialize();
}


// Public API

bool UInspectSubsystem::BeginInspect(AActor* ActorToInspect, APlayerController* RequestingPC)
{
	if (!ActorToInspect || !RequestingPC)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("[UInspectSubsystem::BeginInspect] ActorToInspect or RequestingPC Cannot be null."));
		return false;
	}

	// Only one inspect session can be active at a time.
	// End the current session before starting another.
	if (IsInspecting())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[UInspectSubsystem::BeginInspect] Already inspecting %s; call EndInspect() before "
			"starting a new session."),
			*CurrentSession->GetInspectedComponent()->GetOwner()->GetName());
		return false;
	}

	// Get default settings from Project Settings
	const UInspectSettings* InspectSettings = GetDefault<UInspectSettings>();

	UInspectableComponent* InspectComp = ActorToInspect->FindComponentByClass<UInspectableComponent>();
	if (!InspectComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UInspectSubsystem::BeginInspect] %s has no InspectableComponent."),
		       *ActorToInspect->GetName());
		return false;
	}

	UInspectDataAsset* Data = IInspectable::Execute_GetInspectData(InspectComp);
	if (!Data)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[UInspectSubsystem::BeginInspect] GetInspectData returned null on %s."),
			*InspectComp->GetName()
		);
		return false;
	}

	UPrimitiveComponent* Mesh = IInspectable::Execute_GetInspectMeshOverride(InspectComp);
	if (!Mesh)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[UInspectSubsystem::BeginInspect] No mesh found on %s."),
			*InspectComp->GetName());
		return false;
	}

	UInspectorComponent* FoundInspectPlayerComp = FindInspectPlayerComponent(RequestingPC);
	if (!FoundInspectPlayerComp)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[UInspectSubsystem::BeginInspect] No UInspectorComponent found on PlayerController"
			" nor PlayerCharacter."));
		return false;
	}

	// Cache the player controller and player component 
	OwningPC = RequestingPC;
	InspectPlayerComponent = FoundInspectPlayerComp;
	
	// Pause game if required
	if (InspectSettings->bPauseGameWhileInspection)
	{
		OwningPC->SetPause(true);
	}

	// Notify source actor 
	IInspectable::Execute_OnInspectBegin(InspectComp);

	// Build capture setup 
	SetupCaptureActor(Mesh);

	// Spawn the session 
	UClass* SessionClass = Data->OverrideSessionClass
		                       ? Data->OverrideSessionClass.Get()
		                       : UInspectSession::StaticClass();

	CurrentSession = NewObject<UInspectSession>(this, SessionClass);
	CurrentSession->Initialize(this, InspectComp, Data, OwningPC, InspectMeshProxy);
	CurrentSession->SetZoom(Data->InitialInspectScale);

	CurrentSession->OnSessionStart();

	const TSoftClassPtr<UInspectWidget>& CustomClass = InspectComp->GetCustomWidgetClass();
 
	TSubclassOf<UInspectWidget> InspectWidgetClass =
		CustomClass.IsValid()
			? CustomClass.LoadSynchronous()
			: InspectSettings->InspectWidgetClass.LoadSynchronous();
	
	// Show UI 
	if (InspectWidgetClass && !ActiveWidget)
	{
		ActiveWidget = CreateWidget<UInspectWidget>(RequestingPC, InspectWidgetClass);
		if (ActiveWidget)
		{
			ActiveWidget->Initialize(CurrentSession, RenderTarget);
			ActiveWidget->AddToViewport();
		}
	}

	// Input mode 
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	InputMode.SetHideCursorDuringCapture(false);
	RequestingPC->SetInputMode(InputMode);
	RequestingPC->SetShowMouseCursor(true);
	
	HandleInputMappings(CurrentSession->GetInspectedComponent(), true);
	
	return true;
}

void UInspectSubsystem::EndInspect()
{
	if (!IsInspecting())
	{
		return;
	}
	
	// Unpause game 
	if (GetDefault<UInspectSettings>()->bPauseGameWhileInspection)
	{
		OwningPC->SetPause(false);
	}

	if (CurrentSession->GetInspectedComponent())
	{
		// Remove Input Mappings
		HandleInputMappings(CurrentSession->GetInspectedComponent(), false);

		IInspectable::Execute_OnInspectEnd(CurrentSession->GetInspectedComponent());
	}
	
	// Remove UI 
	if (ActiveWidget)
	{
		ActiveWidget->RemoveFromParent();
		ActiveWidget = nullptr;
	}

	// Teardown capture 
	TeardownCaptureActor();

	// Restore input 
	if (OwningPC)
	{
		OwningPC->SetInputMode(FInputModeGameOnly());
		OwningPC->SetShowMouseCursor(false);
	}
	
	CurrentSession->OnSessionEnd();
	
	// Clear references
	CurrentSession = nullptr;
	OwningPC = nullptr;
	InspectPlayerComponent = nullptr;
}

void UInspectSubsystem::DispatchInput(const UInputAction* SourceInputAction, FInputActionValue Value)
{
	if (!SourceInputAction || !CurrentSession)
	{
		return;
	}

	if (const TSubclassOf<UInspectAction>* FoundActionClass = CurrentInspectActionMap.Find(SourceInputAction))
	{
		if (UInspectAction* FoundAction = CurrentSession->GetOrCreateActionInstance(*FoundActionClass))
		{
			if (FoundAction->CanExecute(CurrentSession))
			{
				FoundAction->Execute(CurrentSession, Value);
			}
		}
	}
}

// Private helpers

void UInspectSubsystem::SetupCaptureActor(UPrimitiveComponent* SourceMesh)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Get default settings from Project Settings
	const UInspectSettings* InspectSettings = GetDefault<UInspectSettings>();
	
	// Spawn the hidden capture host actor 
	FActorSpawnParameters Params;
	Params.ObjectFlags = RF_Transient;
	CaptureActor = World->SpawnActor<AActor>(AActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		Params);

	// Render target 
	RenderTarget = NewObject<UTextureRenderTarget2D>(CaptureActor);
	RenderTarget->InitCustomFormat(
		InspectSettings->RenderTargetWidth,
		InspectSettings->RenderTargetHeight,
		PF_FloatRGBA,
		false);
	RenderTarget->ClearColor = FLinearColor::Black;
	RenderTarget->UpdateResource();

	// Scene capture component 
	SceneCapture = NewObject<USceneCaptureComponent2D>(CaptureActor, TEXT("InspectCapture"));
	SceneCapture->RegisterComponent();
	CaptureActor->SetRootComponent(SceneCapture);

	SceneCapture->PrimaryComponentTick.bTickEvenWhenPaused = true;
	SceneCapture->TextureTarget = RenderTarget;
	SceneCapture->CaptureSource = SCS_SceneColorHDR;
	SceneCapture->bCaptureEveryFrame = true;
	SceneCapture->bCaptureOnMovement = true;
	SceneCapture->ShowFlags.SetAtmosphere(false);
	SceneCapture->ShowFlags.SetFog(false);

	// Position the camera somewhere clean and isolated.
	// Visibility layers can be used to further isolate if needed.
	SceneCapture->SetWorldLocation(FVector(0.0f, 0.0f, 50000.0f));
	SceneCapture->SetWorldRotation(FRotator(0.0f, 0.0f, 0.0f));
	if (InspectSettings->bOverrideCameraFOV)
	{
		SceneCapture->FOVAngle = InspectSettings->CameraFOV; // Tighter FOV = less distortion on items
	}
	
	// Mesh proxy: a duplicate mesh placed in front of the capture camera.
	InspectMeshProxy = CreateMeshProxy(SourceMesh);

	if (InspectMeshProxy)
	{
		InspectMeshProxy->SetRelativeLocation(FVector(100, 0.0f, 0.0f));
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[UInspectSubsystem::SetupCaptureActor] Failed to create mesh "
			"proxy for %s (unsupported mesh type)."),
			*SourceMesh->GetName());
	}
}

void UInspectSubsystem::TeardownCaptureActor()
{
	if (InspectMeshProxy)
	{
		InspectMeshProxy->DestroyComponent();
		InspectMeshProxy = nullptr;
	}
	if (SceneCapture)
	{
		SceneCapture->DestroyComponent();
		SceneCapture = nullptr;
	}
	if (CaptureActor)
	{
		CaptureActor->Destroy();
		CaptureActor = nullptr;
	}
	RenderTarget = nullptr;
}

void UInspectSubsystem::HandleInputMappings(UInspectableComponent* InspectedComponent, bool bAddInspectMappings)
{
	if (!InspectedComponent || !InspectPlayerComponent)
	{
		return;
	}

	const FInspectMapping& ItemMapping = InspectedComponent->GetInspectActionMapping();
	const bool bUseDefault = InspectedComponent->ShouldUseDefaultInspectMapping();

	if (bAddInspectMappings)
	{
		if (bUseDefault && InspectPlayerComponent->DefaultInspectMapping.InputMappingContext)
		{
			InspectPlayerComponent->AddInputMappingContext(
				InspectPlayerComponent->DefaultInspectMapping.InputMappingContext,
				InspectPlayerComponent->DefaultInspectMapping.Priority);
		}

		if (ItemMapping.InputMappingContext)
		{
			InspectPlayerComponent->AddInputMappingContext(
				ItemMapping.InputMappingContext,
				ItemMapping.Priority);
		}

		// Item-specific bindings always win over default bindings for the
		// same UInputAction. This is the sole place this merge happens —
		// UInspectorComponent only ever sees the final, flat result.
		CurrentInspectActionMap = ResolveActionMapping(
			bUseDefault ? InspectPlayerComponent->DefaultInspectMapping.ActionMapping : 
			TMap<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>>(),
			ItemMapping.ActionMapping);

		InspectPlayerComponent->BindActionMapping(CurrentInspectActionMap);
	}
	else
	{
		if (bUseDefault)
		{
			InspectPlayerComponent->RemoveInputMappingContext(
				InspectPlayerComponent->DefaultInspectMapping.InputMappingContext);
		}

		InspectPlayerComponent->RemoveInputMappingContext(ItemMapping.InputMappingContext);

		InspectPlayerComponent->UnbindAllActions();

		CurrentInspectActionMap.Empty();
	}
}

TMap<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>> UInspectSubsystem::ResolveActionMapping(
	const TMap<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>>& DefaultMapping,
	const TMap<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>>& ItemMapping)
{
	TMap<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>> Result = DefaultMapping;

	// Item specific actions, take priority and override
	for (const auto& Pair : ItemMapping)
	{
		Result.Add(Pair.Key, Pair.Value);
	}

	return Result;
}

UPrimitiveComponent* UInspectSubsystem::CreateMeshProxy(UPrimitiveComponent* SourceMesh) const
{
	if (!CaptureActor || !SourceMesh)
	{
		return nullptr;
	}

	USceneComponent* ProxyRoot = CaptureActor->GetRootComponent();

	if (const USkeletalMeshComponent* SrcSkeletal = Cast<USkeletalMeshComponent>(SourceMesh))
	{
		USkeletalMeshComponent* Proxy = NewObject<USkeletalMeshComponent>(CaptureActor);

		Proxy->SetSkeletalMesh(SrcSkeletal->GetSkeletalMeshAsset());

		for (int32 i = 0; i < SrcSkeletal->GetNumMaterials(); ++i)
		{
			Proxy->SetMaterial(i, SrcSkeletal->GetMaterial(i));
		}

		Proxy->SetupAttachment(ProxyRoot);
		Proxy->RegisterComponent();

		return Proxy;
	}

	if (const UStaticMeshComponent* SrcStatic = Cast<UStaticMeshComponent>(SourceMesh))
	{
		UStaticMeshComponent* Proxy = NewObject<UStaticMeshComponent>(CaptureActor);

		Proxy->SetStaticMesh(SrcStatic->GetStaticMesh());

		for (int32 i = 0; i < SrcStatic->GetNumMaterials(); ++i)
		{
			Proxy->SetMaterial(i, SrcStatic->GetMaterial(i));
		}

		Proxy->SetupAttachment(ProxyRoot);
		Proxy->RegisterComponent();

		return Proxy;
	}

	return nullptr;
}

UInspectorComponent* UInspectSubsystem::FindInspectPlayerComponent(const APlayerController* PlayerController) const
{
	if (!PlayerController)
	{
		return nullptr;
	}

	UInspectorComponent* ControllerComponent = 
		PlayerController->FindComponentByClass<UInspectorComponent>();

	UInspectorComponent* PawnComponent = nullptr;
	if (const APawn* Pawn = PlayerController->GetPawn())
	{
		PawnComponent = Pawn->FindComponentByClass<UInspectorComponent>();
	}

	if (ControllerComponent && PawnComponent)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[UInspectSubsystem::FindInspectPlayerComponent] Both PlayerController '%s' and Pawn '%s' have "
			"an InspectPlayerComponent. Only one must have this component."
			"The PlayerController's component will be used."),
			*PlayerController->GetName(),
			*PlayerController->GetPawn()->GetName());

		return ControllerComponent;
	}

	return ControllerComponent ? ControllerComponent : PawnComponent;
}

