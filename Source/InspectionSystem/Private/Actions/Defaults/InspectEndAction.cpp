// Fill out your copyright notice in the Description page of Project Settings.


#include "Actions/Defaults/InspectEndAction.h"

#include "Core/InspectSession.h"

void UInspectEndAction::Execute_Implementation(UInspectSession* InspectSession, FInputActionValue InputValue)
{
	InspectSession->GetInspectSubsystem()->EndInspect();
}
