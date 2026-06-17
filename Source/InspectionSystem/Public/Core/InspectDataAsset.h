// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InspectSession.h"
#include "Engine/DataAsset.h"
#include "InspectDataAsset.generated.h"

/**
 * UInspectDataAsset
 *
 * Drop one of these in your Content Browser for every inspectable item.
 * The InspectableComponent references it. No code changes needed when
 * adding a new item to the game.
 */
UCLASS(BlueprintType)
class INSPECTIONSYSTEM_API UInspectDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
	public:
    	// Display 
    
    	/** Name shown in the inspect UI header. */
    	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
    	FText DisplayName;
    
    	/** Flavor text / lore description shown below the item name. */
    	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
    	FText Description;
	
    	// Core Inspect Behavior
    
    	/**
    	 * Scale applied to the mesh when it first enters inspect mode.
    	 * Tune this so the object fills the screen nicely regardless of its world size.
    	 */
    	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inspect", meta = (ClampMin = "0.01"))
    	float InitialInspectScale = 1.0f;
    
    	/** Min user zoom multiplier. */
    	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inspect", meta = (ClampMin = "0.01"))
    	float MinZoom = 0.5f;

		/** Max user zoom multiplier. */
    	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inspect", meta = (ClampMin = "0.01"))
    	float MaxZoom = 3.0f;
    
    	/** How fast the object rotates in response to drag input (degrees per pixel). */
    	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inspect")
    	float RotationSensitivity = 0.5f;
    
    	/** How fast the object pans (pan is in 2-D screen space, relative to zoom). */
    	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inspect")
    	float PanSensitivity = 1.0f;
    
    	/** Smooth-interpolation speed for all transform changes. 0 = instant. */
    	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inspect", meta = (ClampMin = "0.0"))
    	float InterpSpeed = 12.0f;
	
		// Custom Settings
	
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Session")
		TSubclassOf<UInspectSession> OverrideSessionClass;
};
