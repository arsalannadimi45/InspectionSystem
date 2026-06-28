// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/InspectSubsystem.h"
#include "InspectWidget.generated.h"

class UTextureRenderTarget2D;
class UImage;
class UTextBlock;

/**
 * UInspectWidget
 *
 * C++ base for the full-screen inspect overlay.
 * Create a Blueprint child (WBP_InspectWidget) and bind the named widget
 * properties below to your UMG elements.
 *
 * The C++ base exposes the render target and data asset so you can display
 * everything in Blueprint without needing to touch C++ per-project.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class INSPECTIONSYSTEM_API UInspectWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	/** Image that shows the inspecting item.*/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemImage;
	
public:
	/**
	 * Called by InspectSubsystem immediately after the widget is created.
	 * Passes the render target (for the UImage brush) and the data context.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inspect")
	void InitializeWidget(UInspectSession* InSession, UTextureRenderTarget2D* InRenderTarget);

	
protected:
	
	// Blueprint events 

	/** Called once after InitializeWidget. Override to populate UI elements. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inspect")
	void OnWidgetInitialized(UInspectSession* InSession, UTextureRenderTarget2D* InRenderTarget);

	/** Called once after Render Target Material was created and initialized. Override to modify material instance properties. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inspect")
	void OnRenderMaterialInitialized(UMaterialInstanceDynamic* RenderMaterialInstance);

protected:
	
	
	// Cached data 

	UPROPERTY(BlueprintReadOnly, Category = "Inspect")
	UInspectSession* InspectSession;

	UPROPERTY(BlueprintReadOnly, Category = "Inspect")
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;
	
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> RenderMID;
};
