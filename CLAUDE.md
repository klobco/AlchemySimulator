# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Unreal Engine 5.7 C++ project — an alchemy/crafting simulator. Core loop: player collects plants, brings them to a workbench, uses tools (pestle & mortar, knives) with minigame interactions, and processes ingredients.

The `Source/AlchemySimulator/` module is the active game code. `Variant_Combat/`, `Variant_Platforming/`, and `Variant_SideScrolling/` are experimental sub-systems (separate character/gamemode/controller stacks) and are largely independent of the main alchemy logic.

## Building

Use the VSCode workspace tasks (`.vscode/tasks.json`) — there are 48 pre-configured tasks covering Editor and Game targets in Debug, DebugGame, Development, Test, and Shipping. Typical development uses **Development Editor**.

Direct UBT invocation (from project root):
```
# Build editor (Development)
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" AlchemySimulatorEditor Win64 Development "C:\Users\marti\Documents\Unreal Projects\AlchemySimulator\AlchemySimulator.uproject"
```

No automated test runner is configured. Testing is done by running the editor and playing in-editor.

## Module Dependencies

Declared in [Source/AlchemySimulator/AlchemySimulator.Build.cs](Source/AlchemySimulator/AlchemySimulator.Build.cs):
- `EnhancedInput` — all input handling
- `AIModule`, `StateTreeModule`, `GameplayStateTreeModule` — AI and state machines (used in variants)
- `UMG`, `Slate`, `SlateCore` — all UI widgets
- `GameplayTags` — tag-based classification

## Architecture

### Item System (Data-Asset-Driven)
- `UItemDefinitionBase` — base DataAsset with `EItemCategory` (None, Herb, Ingredient, Tool, Knife, Potion)
- `UPlantItemDefinition` — adds `EIngredientPart` (Whole, Leaf, Stem, Root, Fruit, Flower) for tracking which part of a plant an item represents
- `UToolItemDefinition` — adds workbench cursor texture
- `FItemInstanceData` — runtime data per item instance (currently: Quality)
- `FInventorySlot` — pairs a definition with instance data

### Interaction System
- `IInteractable` — interface (BlueprintNativeEvent): `GetInteractPrompt`, `CanInteract`, `Interact`, `OnFocStart`, `OnFocEnd`
- `UInteractionDetectorComponent` — sphere (220 unit radius) + line-of-sight; tracks one focused target at a time
- `AInteractionCameraRig` — repositions the camera during close interactions

### Inventory System
- `UInventoryComponent` — max 24 slots, optional category whitelist, supports Add/Remove/Transfer/Move; used on character and on station objects
- Workbenches (`ABasicInteractableStationObject`) hold two inventories: `herbsInventory` and `toolsInventory`, plus a `Tools` map and `ActiveToolIndex`

### Plant & World Objects
- `ABasePlant` — composite mesh actor (Stem, Leaf_A, Leaf_B, Fruit), implements `IInteractable`; triggers the cut minigame; transitions between Inventory/OnStand/OnTable states
- `APlantPart` — spawned after cutting; represents a single harvested plant part
- `ABaseTool` / `AToolPestleAndMortar` — tool actors implementing `IInteractable`
- `ABasicWorkbench` — workbench with DropZone/MovingZone, drag-and-drop support for herb placement on the table surface (position clamped to workbench bounds)

### Minigame System
- `UAlchemyMinigameWidget` — base widget; exposes `OnMinigameFinished` delegate
- `UAlchemyCutMinigameWidget` — 20-segment ring with 4 green zones; indicator moves at 0.08 s/segment tick
- `UMinigameManagerComponent` — owns the active minigame widget lifecycle (create, show, destroy)

### UI / Widget Stack
- `UBaseGameWidget` — base for modal widgets; lifecycle: `OnOpened` / `OnClosed` / `CanClose`
- `UWidgetStackManager` — modal stack (Z-order starts at 100); `PushWidget` / `PopWidget` / `CloseAll`; `CanClose` veto prevents stack pop
- `UTableWidget` / `UTableInventoryWidget` — workbench UI
- `UInventoryWidget` / `UInventorySlotWidget` — character inventory UI

### Player Controller
- `AAlchemySimulatorPlayerController` — manages input mapping context, widget stack reference, and drag mechanics (`UInvDragOperation`)

## Key Conventions

- Headers live in `Public/`, implementations in `Private/` under `Source/AlchemySimulator/`
- BlueprintNativeEvent is preferred for interaction callbacks so Blueprint subclasses can override without breaking C++ defaults
- New interactable world objects should implement `IInteractable` and register with `UInteractionDetectorComponent` via overlap
- Item definitions are DataAssets; create them in the editor and reference via soft pointers where load timing matters
- Variant systems (Combat/Platforming/SideScrolling) are self-contained — do not reference main alchemy classes from them
