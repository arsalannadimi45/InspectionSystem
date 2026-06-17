// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/InspectWidget.h"
#include "Components/Image.h"
#include "Styling/SlateBrush.h"
#include "Engine/TextureRenderTarget2D.h"

void UInspectWidget::Initialize(UInspectSession* InSession, UTextureRenderTarget2D* InRenderTarget)
{
	RenderTarget = InRenderTarget;
	InspectSession = InSession; 

	// Project Render Target result on Item Image
	if (ItemImage && InRenderTarget)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(InRenderTarget);
		Brush.ImageSize = FVector2D(InRenderTarget->SizeX, InRenderTarget->SizeY);
		ItemImage->SetBrush(Brush);
	}
	
	// Notify the Blueprint layer
	OnInspectInitialized(InSession, InRenderTarget);
}
