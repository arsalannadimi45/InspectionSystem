// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "InspectSubsystem.h"
#include "InspectAction.generated.h"

class UInspectSession;

UCLASS(Abstract, Blueprintable, EditInlineNew)
class INSPECTIONSYSTEM_API UInspectAction : public UObject
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintNativeEvent)
	void Execute(const UInspectSession* InspectSession, FInputActionValue InputValue);

	UFUNCTION(BlueprintNativeEvent)
	bool CanExecute(const UInspectSession* InspectSession) const;
};
