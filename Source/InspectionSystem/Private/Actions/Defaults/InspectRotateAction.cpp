// Fill out your copyright notice in the Description page of Project Settings.


#include "Actions/Defaults/InspectRotateAction.h"
#include "Core/InspectSession.h"


void UInspectRotateAction::Execute_Implementation(UInspectSession* InspectSession, FInputActionValue InputValue)
{
	// Ignore boolean/button triggers — only act on actual axis delta
	if (InputValue.GetValueType() != EInputActionValueType::Axis2D)
	{
		return;
	}
	
	const FVector2D Delta = InputValue.Get<FVector2D>();
	InspectSession->AddRotationInput(Delta);
}
