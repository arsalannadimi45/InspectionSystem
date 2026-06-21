// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/InspectTypes.h"
#include "Interface/Inspectable.h"
#include "InspectableComponent.generated.h"

class UInputAction;
class UInputMappingContext;
class UInspectDataAsset;

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
	
	// IInspectable 

	virtual UInspectDataAsset* GetInspectData_Implementation() const override;
	virtual void OnInspectBegin_Implementation() override;
	virtual void OnInspectEnd_Implementation() override;
	virtual UPrimitiveComponent* GetInspectMeshOverride_Implementation() const override;
	
protected:
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void RefreshActionMapping();
#endif
	
	// Input Configuration

	/** 
	 * When enabled, this inspectable uses the default inspection input setup
	 * defined in the plugin settings. This includes registering the default
	 * Input Mapping Context and its associated Inspect Action bindings when
	 * inspection begins.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inspect|Input")
	bool bUseDefaultInspectMapping = true;

	/**
	 * Optional object-specific inspection input setup.
	 * Any Input Mapping Contexts and Input Action → Inspect Action bindings
	 * defined here are registered in addition to the default inspection mapping
	 * when this object is inspected.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inspect|Input")
	FInspectMapping AdditionalInspectMapping;
	
	// Generic Data
	
	/** The data asset controlling this object's inspect behavior. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect")
	TObjectPtr<UInspectDataAsset> InspectData;

	/**
	 * If set, this specific mesh component is used during inspection instead
	 * of auto-detecting the first StaticMesh/SkeletalMesh on the owner.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect")
	TObjectPtr<UPrimitiveComponent> MeshOverride;

	// Helpers 

	/**
	 * Finds the best mesh component on the owner actor.
	 * Prefers MeshOverride → first SkeletalMesh → first StaticMesh.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inspect")
	UPrimitiveComponent* ResolveInspectMesh() const;
	
	/**
	* Override this class if you want specific widget class for this object's inspection
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inspect|UI", meta=(AllowPrivateAcess="true"))
	TSoftClassPtr<class UInspectWidget> CustomWidgetClass;
	
	/** Cached resolved mesh so we don't search every frame. */
	UPROPERTY(Transient)
	TObjectPtr<UPrimitiveComponent> CachedMesh;
	
protected:
	virtual void BeginPlay() override;
	
public:	
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inspect|UI")
	TSoftClassPtr<UInspectWidget> GetCustomWidgetClass() { return CustomWidgetClass; }
		
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inspect|Input")	
	const FInspectMapping& GetInspectActionMapping() const { return AdditionalInspectMapping; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inspect|Input")
	bool ShouldUseDefaultInspectMapping() {return bUseDefaultInspectMapping;};
};
