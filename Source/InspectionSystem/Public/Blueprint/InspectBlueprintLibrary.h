// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InspectBlueprintLibrary.generated.h"

class IInspectable;
class UInspectableComponent;

UCLASS()
class INSPECTIONSYSTEM_API UInspectBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category="Inspect", meta =(Keywords="Start", WorldContext="WorldContextObject"))
	static bool BeginInspect(UObject* WorldContextObject, TScriptInterface<IInspectable> Inspectable);
	
	UFUNCTION(BlueprintCallable, Category="Inspect", meta =(Keywords="Finish", WorldContext="WorldContextObject"))
	static void EndInspect(UObject* WorldContextObject);
};
