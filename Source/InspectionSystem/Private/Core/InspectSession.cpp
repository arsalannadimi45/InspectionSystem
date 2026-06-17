// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/InspectSession.h"
#include "Core/InspectDataAsset.h"


void UInspectSession::InitSession_Implementation()
{
	InitialRotation = CurrentRotation;
	InitialZoom     = CurrentZoom;
}
 
void UInspectSession::TerminateSession_Implementation()
{
	// Override in subclasses for custom cleanup.
}

void UInspectSession::AddRotationInput(FVector2D Delta)
{
	const float Sensitivity = Data ? Data->RotationSensitivity : 1.0f;
 
	FRotator NewRotation = CurrentRotation;
	NewRotation.Yaw   += Delta.X * Sensitivity;
	NewRotation.Pitch += Delta.Y * Sensitivity;
 
	SetRotation(NewRotation);
}
 
void UInspectSession::AddPanInput(FVector2D Delta)
{
	const float Sensitivity = Data ? Data->PanSensitivity : 1.0f;
	SetPanOffset(CurrentPanOffset + Delta * Sensitivity);
}
 
void UInspectSession::AddZoomInput(float Delta)
{
	SetZoom(CurrentZoom + Delta);
}
 
void UInspectSession::SetRotation(FRotator NewRotation)
{
	// Clamp pitch so the object can't flip upside down awkwardly.
	NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch, -89.0f, 89.0f);
	NewRotation.Yaw   = FMath::Fmod(NewRotation.Yaw, 360.0f);
	NewRotation.Roll  = 0.0f; // inspect rotation never rolls
 
	CurrentRotation = NewRotation;
	
	ProxyMesh->SetRelativeRotation(CurrentRotation);
}
 
void UInspectSession::SetPanOffset(FVector2D NewOffset)
{
	// Optional safety clamp so users can't drag the object off-screen forever.
	constexpr float MaxPanExtent = 200.0f;
	NewOffset.X = FMath::Clamp(NewOffset.X, -MaxPanExtent, MaxPanExtent);
	NewOffset.Y = FMath::Clamp(NewOffset.Y, -MaxPanExtent, MaxPanExtent);
 
	CurrentPanOffset = NewOffset;
	
	ProxyMesh->SetRelativeLocation(FVector(100.0f, CurrentPanOffset.X, CurrentPanOffset.Y));
}
 
void UInspectSession::SetZoom(float NewZoom)
{
	const float MinZoom = Data ? Data->MinZoom : 0.1f;
	const float MaxZoom = Data ? Data->MaxZoom : 5.0f;
 
	CurrentZoom = FMath::Clamp(NewZoom, MinZoom, MaxZoom);
	
	ProxyMesh->SetRelativeScale3D(FVector(CurrentZoom));
}

void UInspectSession::ResetTransform()
{
	CurrentRotation  = InitialRotation;
	CurrentZoom      = InitialZoom;
	CurrentPanOffset = FVector2D::ZeroVector;
	
	ProxyMesh->SetRelativeLocation(FVector(100.0f, CurrentPanOffset.X, CurrentPanOffset.Y));
	ProxyMesh->SetRelativeRotation(CurrentRotation);
	ProxyMesh->SetRelativeScale3D(FVector(CurrentZoom));

}
