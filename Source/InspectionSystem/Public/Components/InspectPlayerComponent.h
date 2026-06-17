// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "InspectPlayerComponent.generated.h"

class UInputAction;
class UInputMappingContext;
class UInspectSubsystem;
class UInspectTriggerComponent;
class APlayerController;

/**
 * UInspectPlayerComponent
 *
 * Add this to your PlayerPawn (or PlayerCharacter).
 * It manages:
 *   - tracking which inspectable is in range
 *   - showing/hiding the HUD prompt
 *   - forwarding Enhanced Input to the InspectSubsystem
 *
 * The component intentionally contains NO game-specific logic.
 * Swap input actions via the properties panel for each project.
 */
UCLASS( ClassGroup=("Inspect"), meta=(BlueprintSpawnableComponent) )
class INSPECTIONSYSTEM_API UInspectPlayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInspectPlayerComponent();

	// Enhanced Input 
	/** Mapping context added/removed with inspect mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect|Input")
	TObjectPtr<UInputMappingContext> InspectMappingContext;

	/** Priority for the mapping context. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect|Input")
	int32 InspectMappingPriority = 10;

	/** Key that triggers inspect / exits inspect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect|Input")
	TObjectPtr<UInputAction> IA_Interact;

	/** Mouse / stick drag → rotate. Axis2D. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect|Input")
	TObjectPtr<UInputAction> IA_Rotate;

	/** Scroll wheel / triggers → zoom. Axis1D. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect|Input")
	TObjectPtr<UInputAction> IA_Zoom;

	/** Middle-mouse drag / right stick → pan. Axis2D. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect|Input")
	TObjectPtr<UInputAction> IA_Pan;

	/** Reset transform to initial. Button. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect|Input")
	TObjectPtr<UInputAction> IA_ResetTransform;

	/** Close inspect. Button. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect|Input")
	TObjectPtr<UInputAction> IA_CloseInspect;

	// Range tracking (populated by InspectTriggerComponent delegates)

	/** The actor currently in range, if any. Nullptr when out of range. */
	UFUNCTION(BlueprintPure, Category = "Inspect")
	AActor* GetNearestInspectable() const { return NearestInspectable.Get(); }


protected:
	
	virtual void BeginPlay() override;

private:
	// Runtime state
	TWeakObjectPtr<AActor> NearestInspectable;

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> OwningPC;

	// Input callbacks 
	void Input_Interact(const FInputActionValue& Value);
	void Input_Rotate(const FInputActionValue& Value);
	void Input_Zoom(const FInputActionValue& Value);
	void Input_Pan(const FInputActionValue& Value);
	void Input_ResetTransform(const FInputActionValue& Value);
	void Input_CloseInspect(const FInputActionValue& Value);
	
	// Input mapping helpers 

	void AddInspectMappingContext();
	void RemoveInspectMappingContext();
	void BindInspectActions();

	// Helpers 

	UInspectSubsystem* GetInspectSubsystem() const;
};
