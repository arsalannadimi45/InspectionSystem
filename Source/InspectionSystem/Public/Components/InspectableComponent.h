// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/Inspectable.h"
#include "InspectableComponent.generated.h"

class UInspectDataAsset;
class UInspectTriggerComponent;

/**
 * UInspectableComponent
 *
 * Drop this on any Actor to make it inspectable.
 * It provides a default implementation of IInspectable so Blueprints can
 * override individual events without rewriting the whole interface.
 *
 * Pair with UInspectTriggerComponent on the same actor to handle proximity.
 */
UCLASS( ClassGroup=("Inspect"), meta=(BlueprintSpawnableComponent) )
class INSPECTIONSYSTEM_API UInspectableComponent : public UActorComponent, public IInspectable
{
	GENERATED_BODY()

public:	
	
	UInspectableComponent();
	
	// Configuration 
	
	/** The data asset controlling this object's inspect behavior. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect")
	TObjectPtr<UInspectDataAsset> InspectData;

	/**
	 * If set, this specific mesh component is used during inspection instead
	 * of auto-detecting the first StaticMesh/SkeletalMesh on the owner.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect")
	TObjectPtr<UPrimitiveComponent> MeshOverride;

	// IInspectable 

	virtual UInspectDataAsset* GetInspectData_Implementation() const override;
	virtual void OnInspectBegin_Implementation() override;
	virtual void OnInspectEnd_Implementation() override;
	virtual UPrimitiveComponent* GetInspectMeshOverride_Implementation() const override;

	// Helpers 

	/**
	 * Finds the best mesh component on the owner actor.
	 * Prefers MeshOverride → first SkeletalMesh → first StaticMesh.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inspect")
	UPrimitiveComponent* ResolveInspectMesh() const;

protected:
	virtual void BeginPlay() override;

public:	
	/** Cached resolved mesh so we don't search every frame. */
	UPROPERTY(Transient)
	TObjectPtr<UPrimitiveComponent> CachedMesh;
		
};
