#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Actions/InspectAction.h"
#include "InspectTypes.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct FInspectMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Priority = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>> ActionMapping;
};