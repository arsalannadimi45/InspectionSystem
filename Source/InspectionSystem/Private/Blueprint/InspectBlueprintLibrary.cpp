// Fill out your copyright notice in the Description page of Project Settings.


#include "Blueprint/InspectBlueprintLibrary.h"

#include "Core/InspectSubsystem.h"
#include "Interface/Inspectable.h"
#include "Kismet/GameplayStatics.h"

bool UInspectBlueprintLibrary::BeginInspect(UObject* WorldContextObject, TScriptInterface<IInspectable> Inspectable)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject,0);
	
	UInspectSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UInspectSubsystem>(PlayerController->GetLocalPlayer());
	if (!Subsystem) return false; 
	
	return Subsystem->BeginInspect(Inspectable, PlayerController);
}

void UInspectBlueprintLibrary::EndInspect(UObject* WorldContextObject)
{
	const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject,0);

	UInspectSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UInspectSubsystem>(PlayerController->GetLocalPlayer());
	if (!Subsystem) return; 
	
	Subsystem->EndInspect();
}
