// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InspectorComponent.h"
#include "Actions/InspectAction.h"
#include "Core/InspectSubsystem.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

// Sets default values for this component's properties
UInspectorComponent::UInspectorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

#if WITH_EDITOR

void UInspectorComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
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

void UInspectorComponent::RefreshActionMapping()
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

void UInspectorComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		OwnerPlayerController = PC;
	}
	else if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		OwnerPlayerController = Cast<APlayerController>(Pawn->GetController());
	}

	if (!OwnerPlayerController)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[UInspectorComponent::BeginPlay] InspectPlayerComponent must be attached to either"
			" a PlayerController or a Pawn possessed by a PlayerController."));
	}
}

UEnhancedInputLocalPlayerSubsystem* UInspectorComponent::GetInputSubsystem()
{
	if (InputSubsystem)
	{
		return InputSubsystem;
	}

	if (!OwnerPlayerController)
	{
		return nullptr;
	}

	if (const ULocalPlayer* LocalPlayer = OwnerPlayerController->GetLocalPlayer())
	{
		InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	}

	if (!InputSubsystem)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[UInspectorComponent] No UEnhancedInputLocalPlayerSubsystem found on owning PlayerController's LocalPlayer."));
	}

	return InputSubsystem;
}

void UInspectorComponent::AddInputMappingContext(UInputMappingContext* Context, int32 Priority)
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

void UInspectorComponent::RemoveInputMappingContext(UInputMappingContext* Context)
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

void UInspectorComponent::BindActionMapping(const TMap<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>>& ActionMapping)
{
	if (!OwnerPlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UInspectorComponent::BindActionMapping] No OwnerPlayerController, cannot bind."));
		return;
	}

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(OwnerPlayerController->InputComponent);
	if (!EIC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UInspectorComponent::BindActionMapping] OwnerPlayerController has no EnhancedInputComponent."));
		return;
	}

	// Input safety net, if any actions from previous sessions were still bound, unbind and continue
	if (BoundActionHandles.Num() > 0)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[UInspectorComponent::BindActionMapping] Bindings already active; unbinding stale set before rebinding."));
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
			&UInspectorComponent::OnInspectInputTriggered
		);

		BoundActionHandles.Add(Binding.GetHandle());
	}
}

void UInspectorComponent::UnbindAllActions()
{
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(OwnerPlayerController ? OwnerPlayerController->InputComponent : nullptr))
	{
		for (uint32 Handle : BoundActionHandles)
		{
			EIC->RemoveBindingByHandle(Handle);
		}
	}
	BoundActionHandles.Reset();
}

void UInspectorComponent::OnInspectInputTriggered(const FInputActionInstance& ActionInstance)
{
	if (UInspectSubsystem* InspectSubsystem = GetInspectSubsystem())
	{
		InspectSubsystem->DispatchInput(ActionInstance.GetSourceAction(), ActionInstance.GetValue());
	}
}

// Helpers

UInspectSubsystem* UInspectorComponent::GetInspectSubsystem() const
{
	 if (OwnerPlayerController)
	 {
		 return OwnerPlayerController->GetLocalPlayer()->GetSubsystem<UInspectSubsystem>();
	 }
	return nullptr;
}