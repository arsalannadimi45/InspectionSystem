# InspectionSystem

A modular, data-driven object inspection framework for Unreal Engine 5, in the style of Resident Evil's item-examination screens. Drop it into a project, tag actors as inspectable, and get a paused, isolated, rotate/pan/zoom inspection view with almost no per-project C++.

Built with Enhanced Input, `UDataAsset`-driven configuration, and a subsystem-owned architecture designed so that **project code never has to touch or subclass the plugin's C++ to configure new inspectable items** — only to add new custom behavior.

---

## Features

- **Isolated render view** — a duplicated proxy mesh is captured by a dedicated `USceneCaptureComponent2D` into a render target, so the inspected object is displayed cleanly without exposing the actual level geometry behind it.
- **Rotate / pan / zoom** with configurable sensitivity, limits, and smoothing (interpolated, not instant, by default).
- **Data-driven per-item config** (`UInspectConfig`) — initial offset/rotation/scale, pan limits, rotation & zoom sensitivity, zoom clamps, interpolation speed.
- **Composable input mapping** — a project-wide default `Input Action → Inspect Action` map lives on the player, and each inspectable item can add or override entries on top of it, merged automatically per session.
- **Extensible action system** — new inspect behaviors (e.g. "flip page," "toggle flashlight") are added by subclassing `UInspectAction`, no plugin source edits required.
- **Automatic game pausing** during inspection (optional, via project settings), with the render/tick pipeline correctly configured to keep working while paused.
- **UMG integration point** — a `UInspectWidget` base class exposes the render target and session to Blueprint so the on-screen UI is fully re-skinnable.
- **Blueprint-friendly** — `IInspectable` is a `BlueprintNativeEvent` interface, `UInspectableComponent` gives you a ready-made default implementation, and a `UInspectBlueprintLibrary` exposes `BeginInspect`/`EndInspect` as simple static nodes.
- **Ships default content** — default Input Actions, an Input Mapping Context, a render-target material, and a default inspect widget Blueprint are included in the plugin's own Content folder, so a new project works out of the box.

---

## Requirements

- Unreal Engine 5.6+ (C++ project; Blueprint-only projects need to add a C++ class first so the plugin's engine module can compile against it)
- **Enhanced Input** plugin (enabled automatically as a dependency)

---

## Installation

1. Copy the `InspectionSystem` folder into your project's `Plugins/` directory (create the folder if it doesn't exist).
2. Regenerate project files / re-open the project. Unreal will prompt to rebuild modules — accept.
3. Enable the plugin in **Edit → Plugins → Other → InspectionSystem** if it isn't already active.
4. Restart the editor.

---

## Core Concepts

| Class | Role |
|---|---|
| `UInspectSubsystem` | `ULocalPlayerSubsystem`. The single source of truth for the inspect lifecycle. Owns the current `UInspectSession`, the scene capture/render target, the active UI widget, and the resolved input mapping. All other systems talk to it; they never talk to each other directly. |
| `UInspectSession` | A `UObject` created per `BeginInspect` call and destroyed on `EndInspect`. Owns all mutable transform state (rotation, pan, zoom) with clamped mutators and tick-driven interpolation between target and current values. |
| `IInspectable` | The interface any actor must implement to be inspectable. Deliberately minimal — most configuration is delegated to `UInspectConfig`. |
| `UInspectableComponent` | A drop-in `ActorComponent` that implements `IInspectable` for you, with editor-exposed properties for display name/description, config, mesh override, widget override, and per-item input mapping. |
| `UInspectorComponent` | Goes on the `PlayerController` (or `PlayerCharacter`). Holds the project-wide default input mapping, the default `UInspectConfig`, an optional session class override, and routes Enhanced Input events into the subsystem. |
| `UInspectConfig` | A `UDataAsset` describing initial transform offsets and rotate/pan/zoom sensitivity, limits, and smoothing for a class of inspectable items. Reusable across many actors. |
| `UInspectAction` | Base class for a single input-driven behavior during inspection (rotate, pan, zoom, reset, end, or a custom action). Receives the active `UInspectSession` and the raw `FInputActionValue`. |
| `UInspectWidget` | `UUserWidget` base class for the full-screen inspect overlay. Exposes the render target and session to Blueprint. |
| `FInspectMapping` | A struct pairing an `UInputMappingContext` (with priority) with a `TMap<UInputAction*, TSubclassOf<UInspectAction>>`. Used both for the player's default mapping and for any item's additional/override mapping. |

### How a session begins

`UInspectSubsystem::BeginInspect` resolves everything needed to inspect an object in one call:

1. Finds the `UInspectorComponent` on the requesting `PlayerController` (or its Pawn).
2. Resolves the effective `UInspectConfig` — the item's override if set, otherwise the player's default.
3. Resolves the effective inspect mesh — the item's override if set, otherwise the first Static/Skeletal mesh found on the actor.
4. Optionally pauses the game (`PlayerController::SetPause`), if enabled in settings.
5. Spawns a duplicate proxy mesh in front of a dedicated `USceneCaptureComponent2D` and renders it to a `UTextureRenderTarget2D`.
6. Creates the `UInspectSession` (or a project-supplied subclass) and calls `OnSessionStart` / `IInspectable::OnInspectBegin`.
7. Creates and shows the inspect widget (item override, or the project default from settings).
8. Merges the player's default `FInspectMapping` with the item's additional mapping (item entries win on key collision), adds the resulting Input Mapping Context at high priority, and binds all Inspect Actions.

`EndInspect` reverses all of the above and is safe to call even when nothing is active.

---

## Public API

The primary entry points, usable from C++ or Blueprint:

```cpp
// Via the subsystem directly
ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
UInspectSubsystem* InspectSubsystem = LocalPlayer->GetSubsystem<UInspectSubsystem>();
InspectSubsystem->BeginInspect(InspectableInterface, PlayerController);
InspectSubsystem->EndInspect();

// Or via the Blueprint function library (world-context based, no manual subsystem lookup needed)
UInspectBlueprintLibrary::BeginInspect(WorldContextObject, InspectableInterface);
UInspectBlueprintLibrary::EndInspect(WorldContextObject);

// Or directly on the component, if you're using UInspectableComponent
InspectableComponent->BeginInspect();
InspectableComponent->EndInspect();
```

`UInspectableComponent` also broadcasts `OnInspectStarted(UInspectableComponent*, UInspectSession*)` and `OnInspectEnded(UInspectableComponent*)` delegates for reacting elsewhere in Blueprint.

---

## Setup Guide

### 1. Add `UInspectorComponent` to your player

Add the component to your `PlayerController` (or `PlayerCharacter` — both are supported). In its details panel:

- **Default Inspect Mapping** — assign an `Input Mapping Context` (the plugin ships `IMC_DefaultInspect`) and build the `Input Action → Inspect Action` map, e.g.:
  - `IA_DefaultInspect_Rotate` → `UInspectRotateAction`
  - `IA_DefaultInspect_Pan` → `UInspectPanAction`
  - `IA_DefaultInspect_Zoom` → `UInspectZoomAction`
  - `IA_DefaultInspect_ResetTransform` → `UInspectResetTransformAction`
  - `IA_DefaultInspect_End` → `UInspectEndAction`
- **Default Inspect Config** — assign a `UInspectConfig` data asset (the plugin ships `DA_DefaultInspectConfig`, you can create one via **Add → Miscellaneous → Data Asset → Inspect Config**) to act as the fallback for any item that doesn't specify its own.
- **Session Class Override** *(optional)* — assign a `UInspectSession` subclass if you need custom per-session state or behavior.

### 2. Make an actor inspectable

Add an `UInspectableComponent` to the actor. In its details panel:

- Set **Display Name** / **Description**.
- Leave **Use Default Inspect Mapping** checked to inherit the player's default bindings, or uncheck it to rely solely on this item's own mapping.
- Optionally fill in **Additional Inspect Mapping** to add item-specific input bindings (or override a default one, e.g. giving this item a unique action on the same rotate button).
- Optionally assign **Inspect Config Override** if this item needs different sensitivity/limits than the project default.
- Optionally assign **Mesh Override** if the component shouldn't auto-detect the first mesh on the actor.
- Optionally assign **Widget Class Override** for item-specific UI.

Then call `InspectableComponent->BeginInspect()` from wherever your interaction system decides the player has focused this object (e.g. a line-trace interact system on key press).

### 3. Configure the UI and render settings

`UInspectSettings` (**Project Settings → Plugins → Inspect System**) exposes:

- **bPauseGameWhileInspection** — pause the game during inspection (default: true).
- **InspectWidgetClass** — default widget shown when an item doesn't override it.
- **InspectRenderMaterial** — material assigned to the widget's item image, driven by the scene capture's render target.
- **RenderTargetWidth / RenderTargetHeight** — capture resolution.
- **bOverrideCameraFOV / CameraFOV** — optional FOV override for the capture camera.

The plugin ships a working default: `WBP_DefaultInspectWidget`, `M_UI_InspectRenderTarget`, and `IMC_DefaultInspect` with its associated Input Actions, all under the plugin's own Content folder, pre-wired in `Config/DefaultInspectionSystem.ini`.

### 4. (Optional) Custom UI

Subclass `UInspectWidget` in Blueprint (or C++). Bind a `UImage` named `ItemImage` in the widget designer. Override:

- `OnWidgetInitialized(UInspectSession*, UTextureRenderTarget2D*)` — populate item name/description, etc.
- `OnRenderMaterialInitialized(UMaterialInstanceDynamic*)` — tweak the dynamic material instance driving the render target display.

### 5. (Optional) Custom inspect actions

Subclass `UInspectAction` and override `Execute_Implementation` (and `CanExecute_Implementation` if the action should be conditionally blocked):

```cpp
void UMyCustomAction::Execute_Implementation(UInspectSession* InspectSession, FInputActionValue InputValue)
{
    // Custom behavior — e.g. toggle a flashlight attached to the proxy mesh
}
```

Then reference the new class from any `FInspectMapping` (default or per-item) in the editor. No plugin source changes required.

---

## Included Default Actions

| Action | Behavior |
|---|---|
| `UInspectRotateAction` | Applies 2D input as rotation delta via `AddRotationInput`. |
| `UInspectPanAction` | Applies 2D input as pan delta via `AddPanInput`. |
| `UInspectZoomAction` | Applies scalar input as zoom delta via `AddZoomInput`. |
| `UInspectResetTransformAction` | Resets rotation/pan/zoom back to the config's initial state via `ResetTransform`. |
| `UInspectEndAction` | Calls `EndInspect` on the owning subsystem. |

---

## Architecture Notes

- **`UInspectSubsystem` is a `ULocalPlayerSubsystem`**, created automatically per local player and torn down on unload — no manual spawning or registration needed.
- **All mutable inspection state lives on `UInspectSession`**, not scattered across individual actions. Actions read and mutate the session through its clamped setter API; they never hold their own transform state. Because `UInspectSession` implements `FTickableGameObject` with `IsTickableWhenPaused() == true`, transform interpolation continues smoothly even while the game is paused.
- **Input mapping is merged, not replaced.** The subsystem always starts from the player's `DefaultInspectMapping` and layers each item's `AdditionalInspectMapping` on top, with item-specific `Input Action → Inspect Action` entries taking priority on collision. This means most items need zero custom input setup, while special items can override just the actions they need to change.
- **The inspected mesh is never modified or moved.** A duplicate proxy mesh is spawned inside a hidden capture actor and rendered in isolation; the real world-space mesh is left untouched (aside from whatever your `OnInspectBegin`/`OnInspectEnd` Blueprint events choose to do with it, e.g. hiding it).
- **`IInspectable` is implemented on a component, not the actor**, via `UInspectableComponent`. The subsystem resolves it with `TScriptInterface<IInspectable>`, so any actor, component, or object graph can supply the interface as long as something on the actor implements it.

---

## Known Limitations

- **Single-player only in V1.** The subsystem, session, and capture setup are built around one local player. Split-screen and networked multiplayer are not yet supported and are planned for a future version.
- **One inspection session at a time.** Calling `BeginInspect` while already inspecting will fail (logged as a warning) until `EndInspect` is called.
