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

#if WITH_EDITOR

void UInspectPlayerComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
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

void UInspectPlayerComponent::RefreshActionMapping()
{
	DefaultInspectMapping.ActionMapping.Empty();

	if (!DefaultInspectMapping.InputMappingContext)
	{
		return;
	}

	TSet<const UInputAction*> UniqueActions;

	for (const FEnhancedActionKeyMapping& Mapping :
		 DefaultInspectMapping.InputMappingContext->GetMappings())
	{
		if (Mapping.Action)
		{
			UniqueActions.Add(Mapping.Action.Get());
		}
	}

	for (const UInputAction* Action : UniqueActions)
	{
		DefaultInspectMapping.ActionMapping.FindOrAdd(
			const_cast<UInputAction*>(Action));
	}
}
#endif


// Lifecycle

void UInspectPlayerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwningPC = Cast<APlayerController>(GetOwner());
	if (!OwningPC)
	{
		UE_LOG(LogTemp, Error, TEXT("[UInspectPlayerComponent::BeginPlay] InspectPlayerComponent must be attached to the Player Controller."))
	}
}

void UInspectPlayerComponent::AddInputMappingContext(UInputMappingContext* Context, int32 Priority)
{
	if (!Context || !OwningPC)
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
	UE_LOG(LogTemp, Error, TEXT("[UInspectPlayerComponent::AddInputMappingContext] Removed Mapping Context %s"), *Context->GetFullName())
	
}

void UInspectPlayerComponent::BindActionMapping(const TMap<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>>& ActionMapping)
{
	UE_LOG(LogTemp, Warning, TEXT("[UInspectPlayerComponent::BindActionMapping]"));
	
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(OwningPC->InputComponent);
	if (!EIC) return;

	for (const auto& Pair : ActionMapping)
	{
		const UInputAction* IA = Pair.Key;
		if (!IA) continue;

		FEnhancedInputActionEventBinding& Binding = EIC->BindAction(
			IA,
			ETriggerEvent::Triggered,
			this,
			&UInspectPlayerComponent::OnInspectInputTriggered
		);

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
	if (UInspectSubsystem* InspectSubsystem = GetInspectSubsystem())
	{
		InspectSubsystem->DispatchInput(ActionInstance.GetSourceAction(), ActionInstance.GetValue());
	}
}

// Helpers

UInspectSubsystem* UInspectPlayerComponent::GetInspectSubsystem() const
{
	UWorld* World = GetWorld();
	return World ? World->GetSubsystem<UInspectSubsystem>() : nullptr;
}
