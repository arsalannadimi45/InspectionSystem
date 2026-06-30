// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "InputTriggers.h"
#include "Core/InspectTypes.h"
#include "InspectorComponent.generated.h"

struct FInputActionInstance;
class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UInputMappingContext;
class UInspectSubsystem;
class UInspectTriggerComponent;
class APlayerController;

/**
 * UInspectorComponent
 *
 * Add this to your PlayerController (or PlayerCharacter).
 * It handles input routing and default data that should be used across inspections.
 */
UCLASS( ClassGroup=("Inspect"), meta=(BlueprintSpawnableComponent) )
class INSPECTIONSYSTEM_API UInspectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInspectorComponent();
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void RefreshActionMapping();
#endif
	
protected:
	
	// Enhanced Input 

	/**
	 * Default inspection input setup.
	 * Any Input Mapping Contexts and Input Action → Inspect Action bindings
	 * defined here are registered as the default inspection mapping
	 * when this object is inspected.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inspect|Input")
	FInspectMapping DefaultInspectMapping;
	
	/** The default inspect config that's used for all inspectable objects unless overriden. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect|Config")
	TObjectPtr<UInspectConfig> DefaultInspectConfig;
	
	// Session

	/**
	 * Override session class in order to have your own session class when inspection begins.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect|Session")
	TSubclassOf<UInspectSession> SessionClassOverride;

public:
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inspect|Input")
	FORCEINLINE FInspectMapping GetDefaultInspectMapping() const { return DefaultInspectMapping; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inspect|Config")
	FORCEINLINE UInspectConfig* GetDefaultInspectConfig() const { return DefaultInspectConfig; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inspect|Session")
	FORCEINLINE TSubclassOf<UInspectSession> GetSessionClassOverride() const { return SessionClassOverride; }
	
	UFUNCTION(BlueprintCallable, Category = "Inspect|Input")
	void AddInputMappingContext(UInputMappingContext* Context, int32 Priority);
	
	UFUNCTION(BlueprintCallable, Category = "Inspect|Input")
	void RemoveInputMappingContext(UInputMappingContext* Context);
	
	/**
	 * Binds Enhanced Input actions to forward into the InspectSubsystem.
	 * @param ActionMapping  Final, already-resolved map (subsystem is
	 *                       responsible for merging default + per-item maps
	 *                       before calling this). Each binding uses the
	 *                       trigger event declared by the corresponding
	 *                       UInspectAction's CDO.
	 */
	void BindActionMapping(const TMap<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>>& ActionMapping);
 
	/** Unbinds everything bound by the most recent BindActionMapping call. Safe to call repeatedly / when nothing is bound. */
	void UnbindAllActions();
 
	/** True if BindActionMapping has bindings currently active. Used by the subsystem as a re-entrancy guard. */
	bool HasActiveBindings() const { return BoundActionHandles.Num() > 0; }
	
protected:
	
	UPROPERTY(Transient)
	TObjectPtr<APlayerController> OwnerPlayerController;
	
	UPROPERTY()
	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystem;
	
	UPROPERTY(Transient)
	TArray<uint32> BoundActionHandles;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnInspectInputTriggered(const FInputActionInstance& ActionInstance);
 
	/** Lazily resolves InputSubsystem from the owning PlayerController's LocalPlayer. Logs once on failure. */
	UEnhancedInputLocalPlayerSubsystem* GetInputSubsystem();
	
	// Helpers 
 
	UInspectSubsystem* GetInspectSubsystem() const;
};
