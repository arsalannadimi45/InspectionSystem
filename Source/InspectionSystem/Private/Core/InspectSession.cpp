// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/InspectSession.h"

#include "InputAction.h"
#include "Components/InspectableComponent.h"
#include "Core/InspectDataAsset.h"


void UInspectSession::OnSessionStart_Implementation()
{
	InitialRotation = CurrentRotation;
	InitialZoom     = CurrentZoom;
}
 
void UInspectSession::OnSessionEnd_Implementation()
{
	// Override in subclasses for custom cleanup.
}

void UInspectSession::Tick(float DeltaTime)
{
	if (!Data || !ProxyMesh) return;

	const float Speed = Data->InterpSpeed;

	// If InterpSpeed is 0, snap instantly
	if (Speed <= 0.0f)
	{
		CurrentRotation  = TargetRotation;
		CurrentPanOffset = TargetPanOffset;
		CurrentZoom      = TargetZoom;
	}
	else
	{
		CurrentRotation  = FMath::RInterpTo(CurrentRotation,  TargetRotation,  DeltaTime, Speed);
		CurrentPanOffset = FMath::Vector2DInterpTo(CurrentPanOffset, TargetPanOffset, DeltaTime, Speed);
		CurrentZoom      = FMath::FInterpTo(CurrentZoom, TargetZoom, DeltaTime, Speed);
	}
		
	SetPanOffset(CurrentPanOffset);
	SetRotation(CurrentRotation);
	SetZoom(CurrentZoom);
}

void UInspectSession::Initialize(UInspectSubsystem* InSubsystem, UInspectableComponent* InComponent,
                                 UInspectDataAsset* InData, APlayerController* InPC, UPrimitiveComponent* InProxyMesh)
{
	Subsystem = InSubsystem;
	InspectedComponent = InComponent;
	Data = InData;
	OwningPC = InPC;
	ProxyMesh = InProxyMesh;

	check(Subsystem);
	check(InspectedComponent);
	check(OwningPC);
	check(ProxyMesh);
}

void UInspectSession::AddRotationInput(FVector2D Delta)
{
	const float Sensitivity = Data ? Data->RotationSensitivity : 1.0f;

	const float YawDelta   = Delta.X * Sensitivity;
	const float PitchDelta = -Delta.Y * Sensitivity;

	const FQuat YawQuat = FQuat(FVector::UpVector, FMath::DegreesToRadians(YawDelta));

	const FQuat PitchQuat = FQuat(FVector::RightVector, FMath::DegreesToRadians(PitchDelta));

	const FQuat NewRotation =
		YawQuat *
		PitchQuat *
		ProxyMesh->GetRelativeRotation().Quaternion();
	
	TargetRotation = NewRotation.Rotator();
}
 
void UInspectSession::AddPanInput(FVector2D Delta)
{
	const float Sensitivity = Data ? Data->PanSensitivity : 1.0f;
	TargetPanOffset = CurrentPanOffset + Delta * Sensitivity;
}
 
void UInspectSession::AddZoomInput(float Delta)
{
	TargetZoom = FMath::Clamp(TargetZoom + Delta, Data->MinZoom, Data->MaxZoom);
}
 
void UInspectSession::SetRotation(FRotator NewRotation)
{
	ProxyMesh->SetRelativeRotation(NewRotation);
}
 
void UInspectSession::SetPanOffset(FVector2D NewOffset)
{
	// Optional safety clamp so users can't drag the object off-screen forever.
	constexpr float MaxPanExtent = 200.0f;
	NewOffset.X = FMath::Clamp(NewOffset.X, -MaxPanExtent, MaxPanExtent);
	NewOffset.Y = FMath::Clamp(NewOffset.Y, -MaxPanExtent, MaxPanExtent);
	
	ProxyMesh->SetRelativeLocation(FVector(100.0f, NewOffset.X, NewOffset.Y));
}
 
void UInspectSession::SetZoom(float NewZoom)
{
	const float MinZoom = Data ? Data->MinZoom : 0.1f;
	const float MaxZoom = Data ? Data->MaxZoom : 5.0f;
 
	NewZoom = FMath::Clamp(NewZoom, MinZoom, MaxZoom);
	
	ProxyMesh->SetRelativeScale3D(FVector(NewZoom));
}

void UInspectSession::ResetTransform()
{
	TargetRotation  = InitialRotation;
	TargetPanOffset = FVector2D::ZeroVector;
	TargetZoom      = InitialZoom;
}

UInspectAction* UInspectSession::GetOrCreateActionInstance(TSubclassOf<UInspectAction> ActionClass)
{
	if (!ActionClass)
	{
		return nullptr;
	}

	if (TObjectPtr<UInspectAction>* Found = ActionInstances.Find(ActionClass))
	{
		return *Found;
	}

	UInspectAction* NewInstance = NewObject<UInspectAction>(this, ActionClass);
	ActionInstances.Add(ActionClass, NewInstance);
	return NewInstance;
}
