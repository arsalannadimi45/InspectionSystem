#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InspectAction.h"
#include "InspectTypes.generated.h"

USTRUCT(BlueprintType)
struct FInspectMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	UPROPERTY(EditAnywhere)
	int Priority = 100;

	UPROPERTY(EditAnywhere)
	TMap<TObjectPtr<UInputAction>, TSubclassOf<UInspectAction>> ActionMapping;
};