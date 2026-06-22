// Fill out your copyright notice in the Description page of Project Settings.


#include "Actions/Defaults/InspectZoomAction.h"

#include "Core/InspectSession.h"

void UInspectZoomAction::Execute_Implementation(UInspectSession* InspectSession, FInputActionValue InputValue)
{
	const float Delta = InputValue.Get<float>();

	InspectSession->AddZoomInput(Delta);
}
