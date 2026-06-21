// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "InputTriggers.h"
#include "Core/InspectTypes.h"
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
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void RefreshActionMapping();
#endif
	

	// Enhanced Input 

	/**
 	* Default inspection input setup.
 	* Any Input Mapping Contexts and Input Action → Inspect Action bindings
 	* defined here are registered as the default inspection mapping
 	* when this object is inspected.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inspect|Input")
	FInspectMapping DefaultInspectMapping;

public:
	
	UFUNCTION(BlueprintCallable, Category = "Inspect|Input")
	void AddInputMappingContext(UInputMappingContext* Context, int32 Priority);
	
	UFUNCTION(BlueprintCallable, Category = "Inspect|Input")
	void RemoveInputMappingContext(UInputMappingContext* Context);
	
	void BindActionMapping(const TMap<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>>& ActionMapping);
	void UnbindAllActions();
	
protected:
	
	UPROPERTY(Transient)
	TObjectPtr<APlayerController> OwningPC;
	
	UPROPERTY()
	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystem;
	
	UPROPERTY(Transient)
	TArray<uint32> BoundActionHandles;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnInspectInputTriggered(const FInputActionInstance& ActionInstance);
	
	// Helpers 

	UInspectSubsystem* GetInspectSubsystem() const;
};
