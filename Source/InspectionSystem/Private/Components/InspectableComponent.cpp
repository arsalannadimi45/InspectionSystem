// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/InspectableComponent.h"

#include "EnhancedActionKeyMapping.h"
#include "InputMappingContext.h"
#include "Blueprint/InspectBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

#if WITH_EDITOR
void UInspectableComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName =
		PropertyChangedEvent.GetPropertyName();

	if (PropertyName ==
		GET_MEMBER_NAME_CHECKED(FInspectMapping, InputMappingContext))
	{
		RefreshActionMapping();
	}
}

void UInspectableComponent::RefreshActionMapping()
{
	AdditionalInspectMapping.ActionMapping.Empty();

	if (!AdditionalInspectMapping.InputMappingContext)
	{
		return;
	}

	TSet<const UInputAction*> UniqueActions;

	for (const FEnhancedActionKeyMapping& Mapping :
		 AdditionalInspectMapping.InputMappingContext->GetMappings())
	{
		if (Mapping.Action)
		{
			UniqueActions.Add(Mapping.Action.Get());
		}
	}

	for (const UInputAction* Action : UniqueActions)
	{
		AdditionalInspectMapping.ActionMapping.FindOrAdd(
			const_cast<UInputAction*>(Action));
	}
}
#endif


// Called when the game starts
void UInspectableComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Pre-cache the mesh at BeginPlay so ResolveInspectMesh() is O(1) at runtime.
	CachedMesh = ResolveInspectMesh();
}

// IInspectable default implementations 

bool UInspectableComponent::BeginInspect()
{
	TScriptInterface<IInspectable> Inspectable;
	Inspectable.SetObject(this);
	Inspectable.SetInterface(Cast<IInspectable>(this));
	
	return UInspectBlueprintLibrary::BeginInspect(GetWorld(), Inspectable);
}

void UInspectableComponent::EndInspect()
{
	UInspectBlueprintLibrary::EndInspect(GetWorld());
}

void UInspectableComponent::OnInspectBegin_Implementation(UInspectSession* InspectSession)
{
	OnEnterInspect();
	OnInspectStarted.Broadcast(this, InspectSession);
}

void UInspectableComponent::OnInspectEnd_Implementation()
{
	OnExitInspect();
	OnInspectEnded.Broadcast(this);
}

// Helpers 

UPrimitiveComponent* UInspectableComponent::ResolveInspectMesh() const
{
	if (MeshOverride)
	{
		return MeshOverride;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	// Prefer skeletal (more common for detailed items), fall back to static.
	if (USkeletalMeshComponent* Skel = Owner->FindComponentByClass<USkeletalMeshComponent>())
	{
		return Skel;
	}

	if (UStaticMeshComponent* Static = Owner->FindComponentByClass<UStaticMeshComponent>())
	{
		return Static;
	}

	return nullptr;
}


