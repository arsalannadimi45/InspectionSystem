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
 * The InspectableComponent references it. 
 */
UCLASS(BlueprintType)
class INSPECTIONSYSTEM_API UInspectDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	
	// Display

	/** Inspected item's display name. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Display")
	FText DisplayName;

	/** Inspected item's description. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Display", meta=(MultiLine=true))
	FText Description;


	// Initial Transform
	
	
	/**
	 * Initial positional offset applied when inspection begins.
	 *
	 * Allows the item to start slightly higher/lower/closer/further
	 * inside the inspection frame.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect|Initial")
	FVector2D InitialPositionOffset = FVector2D::ZeroVector;

	/**
	 * Initial rotation applied when inspection begins.
	 *
	 * Useful for presenting an object from its most interesting angle
	 * rather than using its world rotation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect|Initial")
	FRotator InitialRotationOffset = FRotator::ZeroRotator;
	
	/**
	 * Scale applied when entering inspect mode.
	 * Allows differently-sized meshes to occupy a similar screen space.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect|Initial", meta=(ClampMin="0.01"))
	float InitialInspectScale = 1.0f;

	// Pan

	/** Movement speed while panning the object. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect|Pan")
	float PanSensitivity = 1.0f;

	/**
	 * Maximum distance the object may be panned from its origin.
	 *
	 * X = Horizontal limit
	 * Y = Vertical limit
	 *
	 * Set to (0,0) to completely disable panning.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect|Pan", meta=(ClampMin="0.0"))
	FVector2D PanLimits = FVector2D(100.f, 100.f);
	
	
	// Rotation

	/** Degrees rotated per unit of input. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect|Rotation")
	float RotationSensitivity = 0.5f;
	
	// Zoom

	/** Minimum user zoom multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect|Zoom", meta=(ClampMin="0.01"))
	float MinZoom = 0.5f;

	/** Maximum user zoom multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect|Zoom", meta=(ClampMin="0.01"))
	float MaxZoom = 3.0f;

	
	// Smoothing

	/**
	 * Interpolation speed used when moving between transform states.
	 *
	 * 0 = Instant.
	 * Higher values = snappier response.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect|Smoothing", meta=(ClampMin="0.0"))
	float InterpSpeed = 25.0f;


	// Session

	/**
	 * Override session class in order to have your own session class when inspection begins.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Session")
	TSubclassOf<UInspectSession> OverrideSessionClass;
};
