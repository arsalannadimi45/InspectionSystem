// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "InputTriggers.h"
#include "InspectPlayerComponent.generated.h"

struct FInputActionInstance;
class UEnhancedInputLocalPlayerSubsystem;
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
	

public:
	
	UFUNCTION(BlueprintCallable, Category = "Inspect|Input")
	void AddInputMappingContext(UInputMappingContext* Context, int32 Priority);
	
	UFUNCTION(BlueprintCallable, Category = "Inspect|Input")
	void RemoveInputMappingContext(UInputMappingContext* Context);
	
	/** Priority for the mapping context. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect|Input")
	int32 InspectMappingPriority = 10;
	
protected:
	
	UPROPERTY(Transient)
	TObjectPtr<APlayerController> OwningPC;
	
	UPROPERTY()
	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystem;
	
	UPROPERTY(Transient)
	TArray<uint32> BoundActionHandles;

protected:
	virtual void BeginPlay() override;
	
	void BindActionsFromContext(UInputMappingContext* Context, ETriggerEvent TriggerEvent = ETriggerEvent::Triggered);
	void UnbindAllActions();

	UFUNCTION()
	void DispatchInput(const UInputAction* InputAction, const FInputActionValue& ActionValue);
	
	UFUNCTION()
	void OnInspectInputTriggered(const FInputActionInstance& ActionInstance);
	
	// Helpers 

	UInspectSubsystem* GetInspectSubsystem() const;
};
