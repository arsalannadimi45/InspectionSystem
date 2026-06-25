// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/InspectWidget.h"
#include "Components/Image.h"
#include "Core/InspectSettings.h"
#include "Styling/SlateBrush.h"
#include "Engine/TextureRenderTarget2D.h"

void UInspectWidget::Initialize(UInspectSession* InSession, UTextureRenderTarget2D* InRenderTarget)
{
	RenderTarget = InRenderTarget;
	InspectSession = InSession;

	if (!ItemImage || !InRenderTarget) return;

	UMaterialInterface* BaseMaterial = GetDefault<UInspectSettings>()->InspectRenderMaterial.LoadSynchronous();
	
	if (!BaseMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("InspectRenderMaterial in Project Settings > Plugins > InspectionSystem cannot be null."))
		return;
	}
	
	// Create material instance
	RenderMID = UMaterialInstanceDynamic::Create(BaseMaterial,this);
	RenderMID->SetTextureParameterValue(TEXT("RenderTarget"), RenderTarget);
	
	// Assign material instance to Item Image
	ItemImage->SetBrushFromMaterial(RenderMID);
	
	// Notify the Blueprint layer
	OnRenderMaterialInitialized(RenderMID);
	OnInspectInitialized(InSession, InRenderTarget);
}
