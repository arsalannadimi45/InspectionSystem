// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InspectSettings.generated.h"

class UInspectWidget;
/**
 * General settings for inspect actions
 */
UCLASS(Config=InspectionSystem, DefaultConfig, meta=(DisplayName="Inspect System"))
class INSPECTIONSYSTEM_API UInspectSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	
	/**
	 * Default widget blueprint that pops up upon inspecting an item
	 */
	UPROPERTY(Config, EditAnywhere, Category="UI")
	TSoftClassPtr<UInspectWidget> InspectWidgetClass;
	
	/**
	* Default material that contains Render Target of Scene Capture while inspecting.
	* This material gets assigned to Item Image in Inspect Widget Blueprint.
	*/
	UPROPERTY(Config, EditAnywhere, Category="UI")
	TSoftObjectPtr<UMaterialInterface> InspectRenderMaterial;
	
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
	
public:
	
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
};
