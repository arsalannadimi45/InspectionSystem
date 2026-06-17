// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/InspectSubsystem.h"

#include "Core/InspectDataAsset.h"
#include "Interface/Inspectable.h"
#include "UI/InspectWidget.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/StaticMesh.h"
#include "Blueprint/UserWidget.h"
#include "Components/InspectableComponent.h"
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
	
	// Get default settings from Project Settings
	const UInspectSettings* InspectSettings = GetDefault<UInspectSettings>();
	
	// Find InspectableComponent in 
	UInspectableComponent* InspectComp = ActorToInspect->FindComponentByClass<UInspectableComponent>();
	if (!InspectComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UInspectSubsystem::BeginInspect] %s has no InspectableComponent."), *ActorToInspect->GetName());
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
			*InspectComp->GetName()
		);
		return false;
	}
	
	
	// Cache the player controller
	OwningPC = RequestingPC;
	

	// Notify source actor 
	IInspectable::Execute_OnInspectBegin(InspectComp);

	// Build capture setup 
	SetupCaptureActor(Mesh);
	
	// Spawn the session 
	UClass* SessionClass = Data->OverrideSessionClass ? Data->OverrideSessionClass.Get() : UInspectSession::StaticClass();

	CurrentSession = NewObject<UInspectSession>(this, SessionClass);
 
	CurrentSession->SourceActor = ActorToInspect;
	CurrentSession->Data        = Data;
	CurrentSession->OwningPC    = OwningPC;
	CurrentSession->ProxyMesh   = InspectMeshProxy;
	CurrentSession->Subsystem   = this;
	CurrentSession->SetZoom(Data->InitialInspectScale);
 
	CurrentSession->InitSession();

	TSubclassOf<UInspectWidget> InspectWidgetClass = InspectSettings->InspectWidgetClass.LoadSynchronous();

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

	return true;
}

void UInspectSubsystem::EndInspect()
{
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

	// Clear references
	OwningPC = nullptr;
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
	Params.Name = FName("InspectCaptureActor");
	Params.ObjectFlags = RF_Transient;
	CaptureActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);

	// Render target 
	RenderTarget = NewObject<UTextureRenderTarget2D>(CaptureActor);
	RenderTarget->InitCustomFormat(InspectSettings->RenderTargetWidth, InspectSettings->RenderTargetHeight, PF_B8G8R8A8, false);
	RenderTarget->ClearColor = FLinearColor::Black;
	RenderTarget->UpdateResource();

	// Scene capture component 
	SceneCapture = NewObject<USceneCaptureComponent2D>(CaptureActor, TEXT("InspectCapture"));
	SceneCapture->RegisterComponent();
	CaptureActor->SetRootComponent(SceneCapture);

	SceneCapture->TextureTarget           = RenderTarget;
	SceneCapture->CaptureSource           = SCS_FinalColorLDR;
	SceneCapture->bCaptureEveryFrame      = true;
	SceneCapture->bCaptureOnMovement      = true;
	SceneCapture->ShowFlags.SetAtmosphere(false);
	SceneCapture->ShowFlags.SetFog(false);

	// Position the camera somewhere clean and isolated.
	// Visibility layers can be used to further isolate if needed.
	SceneCapture->SetWorldLocation(FVector(0.0f, 0.0f, 50000.0f));
	SceneCapture->SetWorldRotation(FRotator(-10.0f, 0.0f, 0.0f));
	SceneCapture->FOVAngle = 35.0f; // Tighter FOV = less distortion on items

	// Mesh proxy 
	// Create a duplicate mesh in front of the capture camera.
	
	InspectMeshProxy = CreateMeshProxy(SourceMesh);

	// Place mesh in front of capture camera
	InspectMeshProxy->SetRelativeLocation(FVector(100.0f, 0.0f, 0.0f));
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

UStaticMeshComponent* UInspectSubsystem::CreateMeshProxy(UPrimitiveComponent* SourceMesh) const
{
	if (!CaptureActor || !SourceMesh)
	{
		return nullptr;
	}

	USceneComponent* ProxyRoot = CaptureActor->GetRootComponent();

	// Static Mesh case
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
