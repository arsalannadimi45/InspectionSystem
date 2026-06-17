// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/InspectDataAsset.h"
#include "Inspectable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UInspectable : public UInterface
{
	GENERATED_BODY()
};

/**
 * IInspectable
 *
 * Implement this on any Actor you want to be inspectable.
 * The InspectSubsystem will call these methods during the inspect lifecycle.
 *
 * Deliberately minimal — most config lives in UInspectDataAsset.
 */
class INSPECTIONSYSTEM_API IInspectable
{
	GENERATED_BODY()

public:
	/**
	 * Returns the data asset driving this object's inspect configuration.
	 * Must be overridden. Return nullptr to abort inspection.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inspect")
	UInspectDataAsset* GetInspectData() const;

	/**
	 * Called the moment the player initiates inspection of this object.
	 * Use it to play a pickup animation, hide the world-space mesh, etc.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inspect")
	void OnInspectBegin();

	/**
	 * Called when the player exits inspect mode.
	 * Restore any state you changed in OnInspectBegin.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inspect")
	void OnInspectEnd();

	/**
	 * Optional: override to provide a custom mesh shown during inspection.
	 * Return nullptr to use the actor's own root static/skeletal mesh component.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inspect")
	UPrimitiveComponent* GetInspectMeshOverride() const;
};
