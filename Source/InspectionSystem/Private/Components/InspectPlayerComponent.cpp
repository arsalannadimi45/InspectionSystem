// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InspectPlayerComponent.h"
#include "Core/InspectSubsystem.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
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
}

void UInspectPlayerComponent::AddInputMappingContext(UInputMappingContext* Context, int32 Priority)
{
	if (!Context)
	{
		return;
	}

	if (!InputSubsystem)
	{
		if (const ULocalPlayer* LocalPlayer = OwningPC->GetLocalPlayer())
		{
			InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		}
		if (!InputSubsystem)
		{
			return;
		}
	}

	InputSubsystem->AddMappingContext(Context, Priority);
}

void UInspectPlayerComponent::RemoveInputMappingContext(UInputMappingContext* Context)
{
	if (!Context)
	{
		return;
	}

	if (!InputSubsystem)
	{
		if (const ULocalPlayer* LocalPlayer = OwningPC->GetLocalPlayer())
		{
			InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		}
		
		if (!InputSubsystem)
		{
			return;
		}
	}

	InputSubsystem->RemoveMappingContext(Context);
}

void UInspectPlayerComponent::BindActionsFromContext(UInputMappingContext* Context, ETriggerEvent TriggerEvent)
{
	if (!Context || !OwningPC)
	{
		return;
	}

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(OwningPC->InputComponent);
	if (!EIC)
	{
		return;
	}

	TSet<const UInputAction*> UniqueActions;
	for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
	{
		if (Mapping.Action)
		{
			UniqueActions.Add(Mapping.Action);
		}
	}

	for (const UInputAction* Action : UniqueActions)
	{
		FEnhancedInputActionEventBinding& Binding =
			EIC->BindAction(Action, TriggerEvent, this, &UInspectPlayerComponent::OnInspectInputTriggered);

		BoundActionHandles.Add(Binding.GetHandle());
	}
}

void UInspectPlayerComponent::UnbindAllActions()
{
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(OwningPC ? OwningPC->InputComponent : nullptr))
	{
		for (uint32 Handle : BoundActionHandles)
		{
			EIC->RemoveBindingByHandle(Handle);
		}
	}
	BoundActionHandles.Reset();
}

void UInspectPlayerComponent::OnInspectInputTriggered(const FInputActionInstance& ActionInstance)
{
	DispatchInput(ActionInstance.GetSourceAction(), ActionInstance.GetValue());
}

void UInspectPlayerComponent::DispatchInput(const UInputAction* InputAction, const FInputActionValue& ActionValue)
{
	// Dispatch Inputs to Subsystem to decide which action should be done
}

// Helpers

UInspectSubsystem* UInspectPlayerComponent::GetInspectSubsystem() const
{
	UWorld* World = GetWorld();
	return World ? World->GetSubsystem<UInspectSubsystem>() : nullptr;
}
