// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actions/InspectAction.h"
#include "InspectEndAction.generated.h"

/**
 * 
 */
UCLASS()
class INSPECTIONSYSTEM_API UInspectEndAction : public UInspectAction
{
	GENERATED_BODY()
	
	virtual void Execute_Implementation(UInspectSession* InspectSession, FInputActionValue InputValue) override;
};
