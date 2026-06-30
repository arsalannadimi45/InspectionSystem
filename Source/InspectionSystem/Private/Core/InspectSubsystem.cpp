// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/InspectSubsystem.h"

#include "Actions/InspectAction.h"
#include "Core/InspectConfig.h"
#include "Interface/Inspectable.h"
#include "UI/InspectWidget.h"
#include "Core/InspectTypes.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Blueprint/UserWidget.h"
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

bool UInspectSubsystem::BeginInspect(TScriptInterface<IInspectable> Inspectable, APlayerController* RequestingPC)
{
	if (!Inspectable || !RequestingPC)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("[UInspectSubsystem::BeginInspect] Inspectable or RequestingPC cannot be null."));
		return false;
	}

	// Only one inspect session can be active at a time.
	// End the current session before starting another.
	if (IsInspecting())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[UInspectSubsystem::BeginInspect] Already inspecting %s; call EndInspect() before "
			"starting a new session."),
			*CurrentSession->GetInspectable().GetObjectRef()->GetName());
		return false;
	}
	
	UInspectorComponent* FoundInspectorComp = FindInspectPlayerComponent(RequestingPC);
	if (!FoundInspectorComp)
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
	InspectorComponent = FoundInspectorComp;
	
	// Get default settings from Project Settings
	const UInspectSettings* InspectSettings = GetDefault<UInspectSettings>();

	// Get resolved inspect config
	UInspectConfig* ConfigOverride = IInspectable::Execute_GetInspectConfig(Inspectable.GetObject());	
	UInspectConfig* Config = ConfigOverride
		? ConfigOverride
		: InspectorComponent->GetDefaultInspectConfig();
	
	if (!Config)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[UInspectSubsystem::BeginInspect] GetInspectConfig returned null on %s. at least "
			"Default Inspect Config on Inspector must be assigned in order to work by a default configuration."),
			*Inspectable.GetObjectRef()->GetName()
		);
		return false;
	}

	UPrimitiveComponent* Mesh = IInspectable::Execute_GetInspectMesh(Inspectable.GetObject());
	if (!Mesh)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[UInspectSubsystem::BeginInspect] No mesh found on %s."),
			*Inspectable.GetObjectRef()->GetName());
		return false;
	}
	
	// Pause game if required
	if (InspectSettings->bPauseGameWhileInspection)
	{
		OwningPC->SetPause(true);
	}

	// Build capture setup 
	SetupCaptureActor(Mesh);

	// Spawn the session 
	const TSubclassOf<UInspectSession> SessionClassOverride { InspectorComponent->GetSessionClassOverride() };
	const UClass* SessionClass = SessionClassOverride
		? SessionClassOverride.Get()
		: UInspectSession::StaticClass();

	CurrentSession = NewObject<UInspectSession>(this, SessionClass);
	CurrentSession->InitializeSession(this, Inspectable, Config, OwningPC, InspectMeshProxy);
	CurrentSession->SetZoom(Config->InitialInspectScale);

	CurrentSession->OnSessionStart();
	
	// Notify source actor 
	IInspectable::Execute_OnInspectBegin(Inspectable.GetObject(), GetCurrentSession());
	
	const TSoftClassPtr<UInspectWidget>& CustomClass = IInspectable::Execute_GetInspectWidgetClass(Inspectable.GetObject()).Get();

	const TSubclassOf<UInspectWidget> InspectWidgetClass =
		CustomClass.IsValid()
			? CustomClass.LoadSynchronous()
			: InspectSettings->InspectWidgetClass.LoadSynchronous();
	
	// Show UI 
	if (InspectWidgetClass && !ActiveWidget)
	{
		ActiveWidget = CreateWidget<UInspectWidget>(RequestingPC, InspectWidgetClass);
		if (ActiveWidget)
		{
			ActiveWidget->InitializeWidget(CurrentSession, RenderTarget);
			ActiveWidget->AddToViewport();
		}
	}

	// Input mode 
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	InputMode.SetHideCursorDuringCapture(false);
	RequestingPC->SetInputMode(InputMode);
	RequestingPC->SetShowMouseCursor(true);
	
	HandleInputMappings(CurrentSession->GetInspectable(), true);
	
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

	if (TScriptInterface<IInspectable> Inspectable = CurrentSession->GetInspectable())
	{
		// Remove Input Mappings
		HandleInputMappings(Inspectable, false);

		IInspectable::Execute_OnInspectEnd(Inspectable.GetObject());
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
	InspectorComponent = nullptr;
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

void UInspectSubsystem::HandleInputMappings(TScriptInterface<IInspectable> Inspectable, bool bAddInspectMappings)
{
	if (!Inspectable || !InspectorComponent)
	{
		return;
	}

	const FInspectMapping& ItemMapping = IInspectable::Execute_GetInspectActionMapping(Inspectable.GetObject());
	const bool bUseDefault = IInspectable::Execute_ShouldAddDefaultInspectMapping(Inspectable.GetObject());

	if (bAddInspectMappings)
	{
		AddInspectMappings(ItemMapping, bUseDefault);
	}
	else
	{
		RemoveInspectMappings(ItemMapping, bUseDefault);
	}
}

void UInspectSubsystem::AddInspectMappings(const FInspectMapping& ItemMapping, bool bUseDefault)
{
	const FInspectMapping& DefaultMapping = InspectorComponent->GetDefaultInspectMapping();
	bool GameWillBePaused = GetDefault<UInspectSettings>()->bPauseGameWhileInspection;
	
	if (bUseDefault && DefaultMapping.InputMappingContext)
	{
		if (GameWillBePaused) ValidateInputMappingContext(DefaultMapping.InputMappingContext);
		
		InspectorComponent->AddInputMappingContext(
			DefaultMapping.InputMappingContext,
			DefaultMapping.Priority);
	}

	if (ItemMapping.InputMappingContext)
	{
		if (GameWillBePaused) ValidateInputMappingContext(ItemMapping.InputMappingContext);
		
		InspectorComponent->AddInputMappingContext(
			ItemMapping.InputMappingContext,
			ItemMapping.Priority);
	}

	// Item-specific bindings override default bindings.
	CurrentInspectActionMap = MergeActionMappings(
		bUseDefault ? DefaultMapping.ActionMapping : TMap<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>>(),
		ItemMapping.ActionMapping);

	InspectorComponent->BindActionMapping(CurrentInspectActionMap);
}

void UInspectSubsystem::RemoveInspectMappings(const FInspectMapping& ItemMapping, bool bUseDefault)
{
	const FInspectMapping& DefaultMapping = InspectorComponent->GetDefaultInspectMapping();

	if (bUseDefault && DefaultMapping.InputMappingContext)
	{
		InspectorComponent->RemoveInputMappingContext(DefaultMapping.InputMappingContext);
	}

	if (ItemMapping.InputMappingContext)
	{
		InspectorComponent->RemoveInputMappingContext(ItemMapping.InputMappingContext);
	}

	InspectorComponent->UnbindAllActions();
	CurrentInspectActionMap.Reset();
}

TMap<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>> UInspectSubsystem::MergeActionMappings(
	const TMap<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>>& DefaultMapping,
	const TMap<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>>& ItemMapping)
{
	TMap<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>> Result(DefaultMapping);

	// Item-specific mappings override defaults.
	for (const TPair<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>>& Pair : ItemMapping)
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
			"an InspectorComponent. Only one must have this component."
			"The PlayerController's component will be used."),
			*PlayerController->GetName(),
			*PlayerController->GetPawn()->GetName());

		return ControllerComponent;
	}

	return ControllerComponent ? ControllerComponent : PawnComponent;
}

void UInspectSubsystem::ValidateInputMappingContext(const UInputMappingContext* MappingContext)
{
	if (MappingContext)
	{
		for (const FEnhancedActionKeyMapping& Mapping : MappingContext->GetMappings())
		{
			if (const UInputAction* InputAction = Mapping.Action)
			{
				 if (!InputAction->bTriggerWhenPaused)
				 {
				 	UE_LOG(LogTemp, Warning, TEXT(
				 		"Inspect Input Action '%s' must have \"Trigger When Paused\" enabled."
				 		" Otherwise, inspect controls will not respond while the game is paused."),
				 		*InputAction->GetName());
				 }
			}
		}
	}
	
}

