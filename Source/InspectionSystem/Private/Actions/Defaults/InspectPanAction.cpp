// Fill out your copyright notice in the Description page of Project Settings.


#include "Actions/Defaults/InspectPanAction.h"

#include "Core/InspectSession.h"

void UInspectPanAction::Execute_Implementation(UInspectSession* InspectSession, FInputActionValue InputValue)
{
	const FVector2D Delta = InputValue.Get<FVector2D>();

	InspectSession->AddPanInput(Delta);
}

