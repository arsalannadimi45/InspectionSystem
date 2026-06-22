// Fill out your copyright notice in the Description page of Project Settings.


#include "Actions/InspectAction.h"
#include "Core/InspectSession.h"

bool UInspectAction::CanExecute_Implementation(UInspectSession* InspectSession) const
{
	return InspectSession != nullptr;
}

void UInspectAction::Execute_Implementation(
	UInspectSession* InspectSession,
	FInputActionValue Value)
{
}