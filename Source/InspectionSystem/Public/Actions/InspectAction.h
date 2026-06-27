// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Core/InspectSubsystem.h"
#include "InspectAction.generated.h"

class UInspectSession;

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew)
class INSPECTIONSYSTEM_API UInspectAction : public UObject
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintNativeEvent)
	bool CanExecute(UInspectSession* InspectSession) const;
	
	UFUNCTION(BlueprintNativeEvent)
	void Execute(UInspectSession* InspectSession, FInputActionValue InputValue);

};
