// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InspectSession.generated.h"

class UInspectableComponent;
class UInspectAction;
class UInspectDataAsset;
class UInspectSubsystem;

/**
 * One instance created per BeginInspect call, destroyed on EndInspect.
 * Owns ALL mutable state for the current inspection: transform, custom
 * per-session data and the proxy mesh reference.
 */

UCLASS(BlueprintType, Blueprintable)
class INSPECTIONSYSTEM_API UInspectSession : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FMyManager, STATGROUP_Tickables);
	}

	virtual bool IsTickable() const override { return true; }
	virtual bool IsTickableWhenPaused() const override { return true; }
	
	UFUNCTION(BlueprintCallable, Category = "Inspect|Session")
	void Initialize(
		UInspectSubsystem* InSubsystem,
		UInspectableComponent* InComponent,
		UInspectDataAsset* InData,
		APlayerController* InPC,
		UPrimitiveComponent* InProxyMesh);

	// Lifecycle (called by subsystem only) 

	/** Called once right after construction, before first action dispatch. */
	UFUNCTION(BlueprintNativeEvent, Category = "Inspect|Session")
	void OnSessionStart();

	/** Called right before the session object is destroyed. */
	UFUNCTION(BlueprintNativeEvent, Category = "Inspect|Session")
	void OnSessionEnd();

	//  Transform functions
	
	void InitializeTransformFromData();

	UFUNCTION(BlueprintCallable, Category = "Inspect|Session|Transform")
	void AddRotationInput(FVector2D Delta);

	UFUNCTION(BlueprintCallable, Category = "Inspect|Session|Transform")
	void AddPanInput(FVector2D Delta);

	UFUNCTION(BlueprintCallable, Category = "Inspect|Session|Transform")
	void AddZoomInput(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Inspect|Session|Transform")
	void SetRotation(FRotator NewRotation);

	UFUNCTION(BlueprintCallable, Category = "Inspect|Session|Transform")
	void SetPanOffset(FVector2D NewOffset);

	UFUNCTION(BlueprintCallable, Category = "Inspect|Session|Transform")
	void SetZoom(float NewZoom);

	UFUNCTION(BlueprintCallable, Category = "Inspect|Session|Transform")
	void ResetTransform();

	// Getters

	UFUNCTION(BlueprintPure, Category="Inspect|Session")
	UInspectSubsystem* GetInspectSubsystem() const { return Subsystem; }

	UFUNCTION(BlueprintPure, Category="Inspect|Session")
	UInspectableComponent* GetInspectedComponent() const { return InspectedComponent; }

	UFUNCTION(BlueprintPure, Category="Inspect|Session")
	UInspectDataAsset* GetInspectData() const { return Data; }

	UFUNCTION(BlueprintPure, Category="Inspect|Session")
	APlayerController* GetOwningPlayerController() const { return OwningPC; }

	UFUNCTION(BlueprintPure, Category="Inspect|Session")
	UPrimitiveComponent* GetProxyMesh() const { return ProxyMesh; }
	
	

	UFUNCTION(BlueprintPure, Category = "Inspect|Session|Transform")
	FRotator GetRotation() const { return CurrentRotation; }

	UFUNCTION(BlueprintPure, Category = "Inspect|Session|Transform")
	FVector2D GetPanOffset() const { return CurrentPanOffset; }

	UFUNCTION(BlueprintPure, Category = "Inspect|Session|Transform")
	float GetZoom() const { return CurrentZoom; }

	UFUNCTION(BlueprintCallable)
	UInspectAction* GetOrCreateActionInstance(TSubclassOf<UInspectAction> ActionClass);
protected:
	
	void UpdateCurrentTransform();
	
protected:
	
	// Context References

	UPROPERTY(BlueprintReadOnly, Category = "Inspect|Session")
	TObjectPtr<UInspectSubsystem> Subsystem;

	UPROPERTY(BlueprintReadOnly, Category = "Inspect|Session")
	TObjectPtr<UInspectableComponent> InspectedComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Inspect|Session")
	TObjectPtr<UInspectDataAsset> Data;

	UPROPERTY(BlueprintReadOnly, Category = "Inspect|Session")
	TObjectPtr<APlayerController> OwningPC;

	UPROPERTY(BlueprintReadOnly, Category = "Inspect|Session")
	TObjectPtr<UPrimitiveComponent> ProxyMesh;

protected:
	
	UPROPERTY()
	FRotator CurrentRotation = FRotator::ZeroRotator;

	UPROPERTY()
	FVector2D CurrentPanOffset = FVector2D::ZeroVector;

	UPROPERTY()
	float CurrentZoom = 1.0f;
	
	FVector2D InitialPanOffset = FVector2D::ZeroVector;

	FRotator InitialRotation = FRotator::ZeroRotator;

	float InitialZoom = 1.0f;

	UPROPERTY()
	TMap<TSubclassOf<UInspectAction>, TObjectPtr<UInspectAction>> ActionInstances;
	
	FRotator TargetRotation  = FRotator::ZeroRotator;
	FVector2D TargetPanOffset = FVector2D::ZeroVector;
	float TargetZoom         = 1.0f;
	
	bool bInitialized = false;
};