// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/InspectConfig.h"
#include "Core/InspectTypes.h"
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
 * Deliberately minimal — most config lives in UInspectConfig.
 */
class INSPECTIONSYSTEM_API IInspectable
{
	GENERATED_BODY()

public:
	/**
	 * @return Display name assigned to this item specifically for inspection
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inspect|Info")
	FText GetDisplayName() const;
	
	/**
	 * @return Description for this item specifically for inspection
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inspect|Info")
	FText GetDescription() const;
	
	/**
	* Returns this object's custom inspect input mapping.
	* Any actions returned here add/override the default mapping for matching input actions.
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inspect|Input")
	FInspectMapping GetInspectActionMapping() const;

	/**
	* Returns whether the project's default inspect input mapping should be applied
	* alongside this object's custom mapping.
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inspect|Input")
	bool ShouldAddDefaultInspectMapping() const;
	
	/**
	 * Returns the data asset driving this object's inspect configuration.
	 * Must be overridden. Return nullptr to abort inspection.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inspect|Config")
	UInspectConfig* GetInspectConfig() const;

	/**
	 * Called the moment the player initiates inspection of this object.
	 * Use it to play a pickup animation, hide the world-space mesh, etc.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inspect")
	void OnInspectBegin(UInspectSession* InspectSession);

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
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inspect|Mesh")
	UPrimitiveComponent* GetInspectMesh() const;
	
	/**
	* Optional: override to create your own Inspect Widget upon inspection.
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inspect|Session")
	TSubclassOf<UInspectWidget> GetInspectWidgetClass() const;
};
