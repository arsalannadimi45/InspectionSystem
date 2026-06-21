// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InspectPlayerComponent.h"
#include "Core/InspectAction.h"
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

UEnhancedInputLocalPlayerSubsystem* UInspectPlayerComponent::GetInputSubsystem()
{
	if (InputSubsystem)
	{
		return InputSubsystem;
	}

	if (!OwningPC)
	{
		return nullptr;
	}

	if (const ULocalPlayer* LocalPlayer = OwningPC->GetLocalPlayer())
	{
		InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	}

	if (!InputSubsystem)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[UInspectPlayerComponent] No UEnhancedInputLocalPlayerSubsystem found on owning PlayerController's LocalPlayer."));
	}

	return InputSubsystem;
}

void UInspectPlayerComponent::AddInputMappingContext(UInputMappingContext* Context, int32 Priority)
{
	if (!Context)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* EI = GetInputSubsystem())
	{
		EI->AddMappingContext(Context, Priority);
	}
}

void UInspectPlayerComponent::RemoveInputMappingContext(UInputMappingContext* Context)
{
	if (!Context)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* EI = GetInputSubsystem())
	{
		EI->RemoveMappingContext(Context);
	}
}

void UInspectPlayerComponent::BindActionMapping(const TMap<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>>& ActionMapping)
{
	if (!OwningPC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UInspectPlayerComponent::BindActionMapping] No OwningPC, cannot bind."));
		return;
	}

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(OwningPC->InputComponent);
	if (!EIC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UInspectPlayerComponent::BindActionMapping] OwningPC has no EnhancedInputComponent."));
		return;
	}

	// Re-entrancy guard: if a previous BeginInspect's bindings weren't torn
	// down (e.g. a missed EndInspect), binding again on top would silently
	// stack duplicate handles and fire actions multiple times per input.
	if (BoundActionHandles.Num() > 0)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[UInspectPlayerComponent::BindActionMapping] Bindings already active; unbinding stale set before rebinding."));
		UnbindAllActions();
	}

	for (const auto& Pair : ActionMapping)
	{
		const UInputAction* IA = Pair.Key;
		const TSubclassOf<UInspectAction> ActionClass = Pair.Value;
		if (!IA || !ActionClass)
		{
			continue;
		}

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