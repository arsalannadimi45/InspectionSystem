// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InspectPlayerComponent.h"
#include "Core/InspectSubsystem.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

// Sets default values for this component's properties
UInspectPlayerComponent::UInspectPlayerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Lifecycle

void UInspectPlayerComponent::BeginPlay()
{
	Super::BeginPlay();

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[InspectPlayerComponent] Owner is not a Pawn — component disabled."));
		return;
	}

	OwningPC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!OwningPC || !OwningPC->IsLocalController())
	{
		// Not the local player — don't set up input.
		return;
	}

	BindInspectActions();
}

// Input actions

void UInspectPlayerComponent::BindInspectActions()
{
	if (!OwningPC)
	{
		return;
	}

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(OwningPC->InputComponent);
	if (!EIC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[InspectPlayerComponent] Enhanced Input Component not found."));
		return;
	}

	// Interaction prompt (world-space, always-on during gameplay)
	if (IA_Interact)     EIC->BindAction(IA_Interact,     ETriggerEvent::Triggered, this, &UInspectPlayerComponent::Input_Interact);

	// These are only meaningful during inspect, but we always bind and guard by subsystem state.
	if (IA_Rotate)       EIC->BindAction(IA_Rotate,       ETriggerEvent::Triggered, this, &UInspectPlayerComponent::Input_Rotate);
	if (IA_Zoom)         EIC->BindAction(IA_Zoom,         ETriggerEvent::Triggered, this, &UInspectPlayerComponent::Input_Zoom);
	if (IA_Pan)          EIC->BindAction(IA_Pan,          ETriggerEvent::Triggered, this, &UInspectPlayerComponent::Input_Pan);
	if (IA_ResetTransform) EIC->BindAction(IA_ResetTransform, ETriggerEvent::Triggered, this, &UInspectPlayerComponent::Input_ResetTransform);
	if (IA_CloseInspect) EIC->BindAction(IA_CloseInspect, ETriggerEvent::Triggered, this, &UInspectPlayerComponent::Input_CloseInspect);
}

// Input callbacks

void UInspectPlayerComponent::Input_Interact(const FInputActionValue& /*Value*/)
{
	UInspectSubsystem* Sub = GetInspectSubsystem();
	if (!Sub)
	{
		return;
	}

	if (Sub->IsInspecting())
	{
		Sub->EndInspect();
		return;
	}

	AActor* Target = NearestInspectable.Get();
	if (Target && OwningPC)
	{
		Sub->BeginInspect(Target, OwningPC);
	}
}

void UInspectPlayerComponent::Input_Rotate(const FInputActionValue& Value)
{

}

void UInspectPlayerComponent::Input_Zoom(const FInputActionValue& Value)
{

}

void UInspectPlayerComponent::Input_Pan(const FInputActionValue& Value)
{

}

void UInspectPlayerComponent::Input_ResetTransform(const FInputActionValue& /*Value*/)
{

}

void UInspectPlayerComponent::Input_CloseInspect(const FInputActionValue& /*Value*/)
{
	if (UInspectSubsystem* Sub = GetInspectSubsystem())
	{
		Sub->EndInspect();
	}
}

// Helpers

UInspectSubsystem* UInspectPlayerComponent::GetInspectSubsystem() const
{
	UWorld* World = GetWorld();
	return World ? World->GetSubsystem<UInspectSubsystem>() : nullptr;
}
