// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InspectAction.h"
#include "Engine/DeveloperSettings.h"
#include "InspectSettings.generated.h"

/**
 * General settings for inspect actions
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Inspect System"))
class INSPECTIONSYSTEM_API UInspectSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	
	
	UPROPERTY(Config, EditAnywhere, Category="Capture")
	int32 RenderTargetWidth = 1920;

	UPROPERTY(Config, EditAnywhere, Category="Capture")
	int32 RenderTargetHeight = 1080;

	UPROPERTY(Config, EditAnywhere, Category="Capture")
	float CameraDistance = 150.f;

	UPROPERTY(Config, EditAnywhere, Category="Capture")
	bool bOverrideCameraFOV = false;

	UPROPERTY(Config, EditAnywhere, Category="Capture", meta=(EditCondition="bOverrideCameraFOV"))
	float CameraFOV = 35.f;

	UPROPERTY(Config, EditAnywhere, Category="UI")
	TSoftClassPtr<class UInspectWidget> InspectWidgetClass;
	
	UPROPERTY(EditAnywhere, Instanced)
	TMap<TObjectPtr<UInputAction>, TObjectPtr<UInspectAction>> DefaultActions;
	
	//UPROPERTY(EditAnywhere, Config)
	TSoftObjectPtr<UInputMappingContext> InspectMappingContext; // = FSoftObjectPath("InspectSystem/Input/IMC_Inspect.IMC_Inspect");
	
	
	
public:
	
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
};
