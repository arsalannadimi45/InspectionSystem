// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/InspectTypes.h"
#include "Interface/Inspectable.h"
#include "InspectableComponent.generated.h"

class UInputAction;
class UInputMappingContext;
class UInspectConfig;

/**
 * UInspectableComponent
 *
 * Drop this on any Actor to make it inspectable.
 * It provides a default implementation of IInspectable so Blueprints can
 * override individual events without rewriting the whole interface.
 *
 * Pair with UInspectTriggerComponent on the same actor to handle proximity.
 */
UCLASS( ClassGroup=("Inspect"), Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent) )
class INSPECTIONSYSTEM_API UInspectableComponent : public UActorComponent, public IInspectable
{
	GENERATED_BODY()

public:	
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInspectionStarted, UInspectableComponent*, InspectedComponent, UInspectSession*, Session);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInspectionEnded, UInspectableComponent*, InspectedComponent);

	UPROPERTY(BlueprintAssignable, Category="Inspect")
	FOnInspectionStarted OnInspectStarted;

	UPROPERTY(BlueprintAssignable, Category="Inspect")
	FOnInspectionEnded OnInspectEnded;
	
	// Public API
	
	UFUNCTION(BlueprintCallable, Category = "Inspect", meta=(Keywords = "Start"))
	bool BeginInspect();
	
	UFUNCTION(BlueprintCallable, Category = "Inspect", meta=(Keywords = "Finish"))
	void EndInspect();
	
	// IInspectable 
	
	virtual FText GetDisplayName_Implementation() const override { return DisplayName; };
	virtual FText GetDescription_Implementation() const override { return Description; };

	virtual void OnInspectBegin_Implementation(UInspectSession* InspectSession) override;
	virtual void OnInspectEnd_Implementation() override;
	
	virtual UInspectConfig* GetInspectConfig_Implementation() const override { return InspectConfigOverride; };
	virtual UPrimitiveComponent* GetInspectMesh_Implementation() const override { return CachedMesh; };
	virtual TSubclassOf<UInspectWidget> GetInspectWidgetClass_Implementation() const override { return WidgetClassOverride; };
	virtual FInspectMapping GetInspectActionMapping_Implementation() const override { return AdditionalInspectMapping; };
	virtual bool ShouldAddDefaultInspectMapping_Implementation() const override { return bUseDefaultInspectMapping; };
	
	
protected:
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void RefreshActionMapping();
#endif
	
	// Inspectable Info

	/** Inspected item's display name. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect|Info")
	FText DisplayName;

	/** Inspected item's description. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspect|Info", meta=(MultiLine=true))
	FText Description;
	
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
	
	// Overriden Data
	
	/** Override this property in order to have a specific Inspect Config for this object. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect|Config")
	TObjectPtr<UInspectConfig> InspectConfigOverride;

	/**
	 * If set, this specific mesh component is used during inspection instead
	 * of auto-detecting the first StaticMesh/SkeletalMesh on the owner.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inspect|Mesh")
	TObjectPtr<UPrimitiveComponent> MeshOverride;

protected:
	
	virtual void BeginPlay() override;
	
	/** Called when inspection begins. Override to implement custom behavior. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent,Category = "Inspect|Inspect")
	void OnEnterInspect();
	
	/** Called when inspection ends. Override to clean up custom behavior. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent,Category = "Inspect|Inspect")
	void OnExitInspect();
	
	// Helpers 

	/**
	 * Finds the best mesh component on the owner actor.
	 * Prefers MeshOverride → first SkeletalMesh → first StaticMesh.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inspect|Mesh")
	UPrimitiveComponent* ResolveInspectMesh() const;
	
	/**
	* Override this class if you want specific widget class for this object's inspection
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inspect|UI", meta=(AllowPrivateAcess="true"))
	TSubclassOf<class UInspectWidget> WidgetClassOverride;
	
	/** Cached resolved mesh so we don't search every frame. */
	UPROPERTY(Transient)
	TObjectPtr<UPrimitiveComponent> CachedMesh;
	
public:	
	
	UFUNCTION(BlueprintCallable, Category = "Inspect|Info")
	void SetInfo(const FText InDisplayName,const FText InDescription)
	{
		DisplayName = InDisplayName;
		Description = InDescription;
	}
	UFUNCTION(BlueprintCallable, Category = "Inspect|Info")
	void SetDisplayName(const FText InDisplayName)
	{
		DisplayName = InDisplayName;
	}
	UFUNCTION(BlueprintCallable, Category = "Inspect|Info")
	void SetDescription(const FText InDescription)
	{
		Description = InDescription;
	}
};
