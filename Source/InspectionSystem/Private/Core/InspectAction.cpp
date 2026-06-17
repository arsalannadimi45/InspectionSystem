// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/InspectAction.h"

#include "Core/InspectSession.h"

bool UInspectAction::CanExecute_Implementation(const UInspectSession* InspectSession) const
{
	return true;
}

void UInspectAction::Execute_Implementation(
	const UInspectSession* InspectSession,
	FInputActionValue Value)
{
}