// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "InspectSubsystem.generated.h"

class UInspectAction;
class UInspectableComponent;
class UInspectPlayerComponent;
class UInspectSession;
struct FInputActionValue;
class UInputAction;
class UInspectDataAsset;
class UInspectWidget;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UStaticMeshComponent;


/**
 * UInspectSubsystem
 *
 * All other systems (player component, UI, etc.) talk to this — they never
 * talk to each other directly. This is the single source of truth.
 *
 * Lifetime: created automatically when the World loads, destroyed when it unloads.
 * Access: UGameplayStatics or UWorld::GetSubsystem<UInspectSubsystem>()
 */


UCLASS()
class INSPECTIONSYSTEM_API UInspectSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Public API 

	/**
	 * Begin inspecting an actor.
	 * @param ActorToInspect  The world actor implementing IInspectable.
	 * @param RequestingPC    The player controller making the request.
	 * @return false if inspection cannot start (already active, missing data, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Inspect")
	bool BeginInspect(AActor* ActorToInspect, APlayerController* RequestingPC);

	/**
	 * End the current inspection session.
	 * Safe to call even when Idle.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inspect")
	void EndInspect();

	UFUNCTION(BlueprintPure, Category = "Inspect")
	UInspectSession* GetCurrentSession() const { return CurrentSession; }


	UFUNCTION(BlueprintPure, Category = "Inspect")
	bool IsInspecting() const { return CurrentSession != nullptr; }
	
	UFUNCTION(BlueprintCallable, Category = "Inspect")
	void DispatchInput(const UInputAction* SourceInputAction, FInputActionValue Value);

protected:
	
	// USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	
	// Necessary references
	
	UPROPERTY(Transient)
	TObjectPtr<UInspectSession> CurrentSession;
	
	UPROPERTY(Transient)
	TObjectPtr<APlayerController> OwningPC;
	
	UPROPERTY(Transient)
	TObjectPtr<UInspectPlayerComponent> InspectPlayerComponent;
	
	UPROPERTY(Transient)
	TObjectPtr<UInspectWidget> ActiveWidget;
	
	TMap<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>> CurrentInspectActionMap;

	// Scene capture setup for the isolated render 

	/** Hidden actor that holds the scene capture + inspect mesh copy. */
	UPROPERTY(Transient)
	TObjectPtr<AActor> CaptureActor;

	UPROPERTY(Transient)
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	/** A duplicate StaticMesh placed in front of the capture camera. */
	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> InspectMeshProxy;

	// Helpers 

	void SetupCaptureActor(UPrimitiveComponent* SourceMesh);
	void TeardownCaptureActor();
	
	void HandleInputMappings(UInspectableComponent* InspectedComponent, bool AddInspectMappings);
	
	UStaticMeshComponent* CreateMeshProxy(UPrimitiveComponent* SourceMesh) const;
	
};
