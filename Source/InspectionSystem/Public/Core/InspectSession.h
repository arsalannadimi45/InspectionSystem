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
class INSPECTIONSYSTEM_API UInspectSession : public UObject
{
	GENERATED_BODY()
	
public:
	
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
	

public:
	
	// Lifecycle (called by subsystem only) 

	/** Called once right after construction, before first action dispatch. */
	UFUNCTION(BlueprintNativeEvent, Category = "Inspect|Session")
	void InitSession();

	/** Called right before the session object is destroyed. */
	UFUNCTION(BlueprintNativeEvent, Category = "Inspect|Session")
	void TerminateSession();
	
	//  Transform functions
	
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
	
	UFUNCTION(BlueprintPure, Category = "Inspect|Session|Transform")
	FRotator GetRotation() const { return CurrentRotation; }
 
	UFUNCTION(BlueprintPure, Category = "Inspect|Session|Transform")
	FVector2D GetPanOffset() const { return CurrentPanOffset; }
 
	UFUNCTION(BlueprintPure, Category = "Inspect|Session|Transform")
	float GetZoom() const { return CurrentZoom; }

	UFUNCTION(BlueprintCallable)
	UInspectAction* GetOrCreateActionInstance(TSubclassOf<UInspectAction> ActionClass);

private:
	
	UPROPERTY()
	FRotator CurrentRotation = FRotator::ZeroRotator;
 
	UPROPERTY()
	FVector2D CurrentPanOffset = FVector2D::ZeroVector;
 
	UPROPERTY()
	float CurrentZoom = 1.0f;
 
	FRotator InitialRotation = FRotator::ZeroRotator;
	
	float InitialZoom = 1.0f;
	
	UPROPERTY()
	TMap<TSubclassOf<UInspectAction>, TObjectPtr<UInspectAction>> ActionInstances;
};
