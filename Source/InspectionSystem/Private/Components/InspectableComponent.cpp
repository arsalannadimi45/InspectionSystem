// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/InspectableComponent.h"
#include "Core/InspectDataAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

UInspectableComponent::UInspectableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UInspectableComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Pre-cache the mesh at BeginPlay so ResolveInspectMesh() is O(1) at runtime.
	CachedMesh = ResolveInspectMesh();
}

// IInspectable default implementations 

UInspectDataAsset* UInspectableComponent::GetInspectData_Implementation() const
{
	return InspectData;
}

void UInspectableComponent::OnInspectBegin_Implementation()
{
	// Default: do nothing. Blueprints can override to hide world mesh,
	// play VO, update quest state, etc.
}

void UInspectableComponent::OnInspectEnd_Implementation()
{
	// Default: do nothing.
}

UPrimitiveComponent* UInspectableComponent::GetInspectMeshOverride_Implementation() const
{
	return CachedMesh;
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


