# OpenWolf

@.wolf/OPENWOLF.md

This project uses OpenWolf for context management. Read and follow .wolf/OPENWOLF.md every session. Check .wolf/cerebrum.md before generating code. Check .wolf/anatomy.md before reading files.


# Design Vault (outside this repo)

Game **content design** — substances, effects, diseases, herbs, balance reasoning — lives in an Obsidian vault at:

```
C:\Users\marti\Obsidian\AlchemyGame
```

It is its own git repository, separate from this one. Commit design changes there as you would code.

- Read `00 Home.md` there before any task about alchemy content, balance, diseases, or "what should this substance do".
- The split is strict: **the vault owns design intent, this repo owns implementation.** Architecture and file layout stay in `CLAUDE.md` / `.wolf/anatomy.md`; do not copy them into the vault, and do not copy design content into `.wolf/`.
- Vault frontmatter keys mirror C++ `UPROPERTY` names exactly, so a note maps 1:1 onto a data asset's fields.
- `01 Systems/*.md` are **transcriptions of this repo's C++** and carry a `source:` key naming the file. If you change that source file, update the note in the same task — a stale design doc is worse than none.
- Content notes are generated from the engine by `Content/Python/dump_alchemy_vault.py` (see "Regenerating vault content" below). Do not hand-edit their frontmatter; it will be overwritten.
- The vault is **not** version controlled. Be careful with destructive edits there.


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

## Regenerating vault content

`Content/Python/dump_alchemy_vault.py` reads the alchemy data assets out of the editor and writes their values into the design vault (see "Design Vault" above). `.uasset` is binary, so this script is the **only** way to get real asset values into the vault — never hand-transcribe them, and never guess them.

Requires the `PythonScriptPlugin` (enabled in the `.uproject`). From the editor: Output Log → console dropdown → **Python**:

```python
import dump_alchemy_vault
dump_alchemy_vault.run()          # add dry_run=True to preview
```

After editing the script, reload it: `import importlib; importlib.reload(dump_alchemy_vault)`.

**Headless (no GUI) — this is how Claude can run it without you:**

```bash
"C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" \
  "<project>/AlchemySimulator.uproject" \
  -run=pythonscript -script="import dump_alchemy_vault; dump_alchemy_vault.run()" \
  -unattended -nosplash -nopause -nullrhi -stdout
```

Takes ~30s. `-nullrhi` skips the renderer, `-unattended` prevents dialogs. **Close the editor first** — two processes on one project contend over the asset registry and DDC. This is a read-only load plus a vault write; it does not build, and does not modify any `.uasset`.

What it touches, and what it must never touch:
- It rewrites **only** the YAML frontmatter and the block between `<!-- generated:begin -->` / `<!-- generated:end -->`. All other prose is preserved byte-for-byte — this is the contract that makes the vault safe to regenerate, so keep it if you modify the script.
- Notes are matched to assets by the `asset:` frontmatter key, not by filename, so vault notes can be freely renamed or moved.
- Files starting with `_` are templates and are skipped, as are `asset:` values that are a bare prefix (`DA_Substance_`).
- Missing notes are created; nothing is ever deleted.

Covered classes: `UDataAssetSubstanceDefinition`, `UDataAssetAlchemyEfectDefinition`, `UDataAssetProcessingMethod`, `UDataAssetDisease`, `UDataAssetPlantPart`. Adding a new alchemy data-asset class means adding an entry to `TYPES` in the script. `DA_Herb_Mint` is a `UPlantItemDefinition` and is **not** covered — its vault note is hand-maintained.

A successful run writes `.sync-state.json` into the vault. `Tools/vault_check.py` reads it to detect when the vault has fallen behind the editor.

## Staying in sync

Four stores hold project state and they drift independently: **code** (`Source/`), **data assets** (`Content/.../DataAssets/`, binary), the **design vault** (outside this repo), and the **OpenWolf logs** (`.wolf/`).

```
python Tools/vault_check.py
```

Answers "is anything out of sync?" in under a second with no editor. Exit 0 = clean, 1 = drift. It never modifies anything. It checks four directions:

1. **code → vault** — a `01 Systems/*.md` note whose `source:` file was edited after the note was written. That note is now lying about the code.
2. **assets → vault** — a `.uasset` newer than the vault's `.sync-state.json`. Re-run the dump.
3. **coverage** — assets with no note, and notes pointing at deleted assets.
4. **git → OpenWolf** — commits or uncommitted work newer than the last `.wolf/memory.md` entry, i.e. work Claude has no record of.

Run it at the start of a session, before a handoff, and after any solo work.

Two slash commands wrap the workflows: **`/catchup`** (start of session — reconstruct unlogged work and repair drift) and **`/handoff`** (end of session — write the record and verify clean).

Full process documentation for all three development modes (solo / assisted / full-Claude) is in [WORKFLOW.md](WORKFLOW.md).

## Module Dependencies

Declared in [Source/AlchemySimulator/AlchemySimulator.Build.cs](Source/AlchemySimulator/AlchemySimulator.Build.cs):
- `EnhancedInput` — all input handling
- `AIModule`, `StateTreeModule`, `GameplayStateTreeModule` — AI and state machines (used in variants)
- `UMG`, `Slate`, `SlateCore` — all UI widgets
- `GameplayTags` — tag-based classification

## Architecture

### Folder Layout (Source/AlchemySimulator/)
Most gameplay code has been reorganized out of the old flat `Public/`/`Private/` split into domain folders where `.h` and `.cpp` live side by side:
- `Actors/Plants/`, `Actors/Tools/`, `Actors/Stations/`, `Actors/Potions/` — world actors
- `Actors/DayNightCycleController.*` — sits directly under `Actors/`
- `Components/Inventory/`, `Components/Minigame/`, `Components/Processing/`, `Components/Disease/`
- `Widgets/Tables/`, `Widgets/Minigames/`, `Widgets/Menu/`, `Widgets/Inventory/`, plus `Widgets/CustomCursorWidget.*` and `Widgets/WidgetStackManager.*` directly under `Widgets/`
- `DataAssets/` — all `UPrimaryDataAsset` alchemy definitions
- `ItemDefinitions/` — all `UItemDefinitionBase` subclasses
- `Subsystems/` — GameInstance/World subsystems
- `Controllers/` — AI controllers (`Characters/` holds NPC pawns)
- `StateTree/Tasks/` — custom StateTree task nodes
- `Public/` / `Private/` at the module root is now reserved for cross-cutting, dependency-free headers used everywhere: `IInteractable`, `IUsebale`, `ItemMetadata.h`, `DataStructHelpers.h`, `InteractionCameraRig`, `InvDragOperation`
- Root-level (no subfolder): `AlchemySimulatorCharacter`, `AlchemySimulatorPlayerController`, `AlchemySimulatorGameMode`, `AlchemySimulator.h`

When adding a new class, match its domain folder rather than reviving `Public/`/`Private/`.

### Item System (Data-Asset-Driven)
- `UItemDefinitionBase` (`ItemDefinitions/`) — base `UPrimaryDataAsset` with `EItemCategory` (None, Herb, Ingredient, Tool, Potion) plus stacking, value, weight. Declares the generic use pipeline: `CanUseItem(FItemUseContext)` / `UseItem(FItemUseContext)` as `BlueprintNativeEvent`s (default implementation just returns failure/false — subclasses opt in). The `Item|World` block (`WorldItem`, `WorldScale`, `WorldTableRotation`) owns how the item appears in the world — `WorldItem` is a *shared generic* Blueprint, never one per species, and the scale/rotation the workbench applies come from here rather than being hardcoded
- `UPlantItemDefinition` — adds `EIngredientPart` (Whole, Leaf, Stem, Root, Fruit, Flower)
- `UToolItemDefinition` — adds workbench cursor texture
- `UPotionItemDefinition` — `Category = Potion`, non-stackable (`MaxStackSize = 1`); overrides `UseItem_Implementation` to apply the potion to a target's `UPatientConditionComponent` (see Disease System) and consumes one item on success
- `DataAssets/DataAssetPlantPart` — also an `UItemDefinitionBase` subclass; the alchemy-facing counterpart to a harvested plant part (substances it yields, base quality, allowed processing tags), plus its `WorldMesh`/`WorldMaterialOverride` and its `AcceptedToolActions`. **Which tools reach a part is per-part data, not per-actor** — that is what lets one species need a knife for its flower and pliers for its spines, and what makes a single shared `BP_Plant` viable
- `FPlantPartHarvestData` (`Public/DataStructHelpers.h`) — one row of a plant's anatomy: which part, how many (`InstanceCount`), where they sit (`AttachSockets` preferred, `RelativeTransforms` as fallback), what cutting one yields (`MinYield`/`MaxYield`/`HarvestChance`), and `bIsStructural` for the trunk/stem. Exactly one row per species should be structural; it is never cut directly and is awarded once every other part is gone. **Sockets on the structural mesh are the intended placement path** — the layout belongs to the mesh asset, not to typed-in coordinates
- `FItemInstanceData` (`ItemMetadata.h`) — runtime per-instance data: `Quality`, `Freshness`, `ProcessingTags`, `ProcessingQuality`, `bIsProcessed`, `bIsPotion`, `RuntimeTags`, and an embedded `FPotionResult` when the instance is a brewed potion
- `FInventorySlot` — pairs a definition with `FItemInstanceData` + quantity
- `FItemUseContext` / `FItemUseResult` (`ItemMetadata.h`) — generic parameter/result pair for the use pipeline: context carries the user actor, its `UInventoryComponent`, slot index/data, and an optional `TargetActor`; result carries success, whether to consume one item, and a message

### Generic "Use Item" Pipeline
- Any item can be "used" via `UItemDefinitionBase::UseItem(FItemUseContext)`; the potion pipeline is the first concrete implementation (drink/administer → treat patient → consume item)
- `IUsebale` (`Public/IUsebale.h`, deliberately spelled this way in-repo) — separate BlueprintNativeEvent interface (`Use(APawn* By)`) for world actors that want a simple "use me" affordance distinct from the inventory-item pipeline above
- `FSTT_UseItemTask` (`StateTree/Tasks/STT_UseItem.*`) — custom StateTree AI task; instance data holds an `AAIController*` and an `FInventorySlot` so NPC behavior trees can trigger the same use pipeline (e.g., an NPC drinking a potion)

### Interaction System
- `IInteractable` — interface (BlueprintNativeEvent): `GetInteractPrompt`, `GetInteractWorldLocation`, `CanInteract`, `Interact`, `OnFocStart`, `OnFocEnd`
- `UInteractionDetectorComponent` — sphere (220 unit radius) + line-of-sight; tracks one focused target at a time
- `AInteractionCameraRig` — repositions the camera during close interactions

### Inventory System
- `UInventoryComponent` (`Components/Inventory/`) — max 24 slots, optional category whitelist, supports Add/Remove/Transfer/Move; used on character and on station objects
- Workbenches (`ABasicInteractableStationObject`) hold two inventories: `herbsInventory` and `toolsInventory`, plus a `Tools` map and `ActiveToolIndex`

### Plant & World Objects
- `ABasePlant` (`Actors/Plants/`) — **data-driven composite actor**, implements `IInteractable` + `IToolActionTarget`; triggers the cut minigame; transitions between Inventory/OnStand/OnTable states. It has exactly one compiled-in component, the root `Body` (the structural stem/stipe/trunk, and the only simulating physics body). Every harvestable part is a `UStaticMeshComponent` created at runtime by `RebuildPartComponents()` from `UPlantItemDefinition::HarvestableParts` — so plant anatomy is data, not a Blueprint hierarchy, and one `BP_Plant` serves every species. `InitializeFromDefinition(Def, Instance)` is the runtime entry point; `OnConstruction` calls the same rebuild so the editor viewport assembles the plant as soon as `Item` is set
- **Never use instanced static meshes for repeated parts** (5 petals). `IToolActionTarget` is keyed on `UPrimitiveComponent* HitComponent`, and an ISM reports one component plus an instance index — per-part cutting would stop working
- Generated part components carry the `GeneratedPlantPart` component tag. Teardown scans for that tag rather than trusting the `Transient` tracking array, which does not survive the actor duplication that happens when a level-placed plant enters PIE
- `APlantPart` (`Actors/Plants/`) — spawned after cutting; represents a single harvested plant part. Its mesh/material/scale come from its `UDataAssetPlantPart` via `SetPartDefinition()`, so one `BP_PlantPart` serves every part in the game
- `ABaseTool` / `AToolPestleAndMortar` (`Actors/Tools/`) — tool actors implementing `IInteractable`
- `ABasicWorkbench` (`Actors/Stations/`) — workbench with DropZone/MovingZone, drag-and-drop support for herb placement on the table surface (position clamped to workbench bounds)
- **Keeping items on the table** — `ABasicWorkbench::ClampActorToWorkbench(Actor, DesiredLocation)` is the one clamp helper; it shrinks the `MovingZone` extents by the actor's own bounds and corrects for the pivot-to-bounds-centre offset, so the *whole mesh* stays on the table rather than just the pivot (clamping the pivot alone lets items overhang the edge). It deliberately does **not** clamp Z — the drag plane sets height and physics settles the item. Three callers: `ABasePlant::Tick`, `APlantPart::Tick` (post-release backstop, guarded on `IsSimulatingPhysics()`), and `AAlchemySimulatorPlayerController::PlayerTick` (during-drag, type-agnostic — it clamps whatever `DraggedActor` is). If items snap to the table centre, `MovingZone` is smaller than the item bounds and needs resizing in the workbench BP
- `ABasePotion` (`Actors/Potions/`) — world potion actor (`PotionMesh` root + attached `LiquidMesh`), implements `IInteractable`; picking it up adds `Item`/`Instance` to the player's inventory and destroys the world actor (mirrors the plant pickup pattern)

### Alchemy Calculation & Substance System (DataAssets/)
Fully data-driven brewing pipeline, computed by `UAlchemyCalculationSubsystem` (`Subsystems/`, a `UGameInstanceSubsystem`):
- `UDataAssetSubstanceDefinition` — a chemical substance: base alchemy effects, base toxicity, stability under heat/drying/crushing, tag-based properties, a color hint (feeds potion liquid color)
- `UDataAssetAlchemyEfectDefinition` — an effect: display info + flags `bIsPositive` / `bIsRootCauseTreatment` / `bIsSymptomRelief` / `bIsSideEffect` (drives disease-treatment matching, not just flavor text)
- `UDataAssetProcessingMethod` — a processing step (e.g. crushing, drying): potency/toxicity/stability multipliers, per-effect modifiers (`FProcessingEffectModifier`), and substance transformation rules (`FSubstanceTransformationRule`, e.g. substance A → substance B with a chance and amount multiplier)
- `UProcessingComponent` (`Components/Processing/`) — `BuildProcessedInstance(originalInstance, processingMethod, minigameQuality)` turns a raw `FItemInstanceData` into a processed one by applying a `UDataAssetProcessingMethod`
- `UAlchemyCalculationSubsystem::CalculatePotionFromSlots(TArray<FInventorySlot>)` — combines processed ingredient slots into a final `FPotionResult` (effects, toxicity, purity, stability, potion color)
- Shared alchemy structs all live in `Public/DataStructHelpers.h` (not `ItemMetadata.h`): `FMinigameResult`, `FAlchemyEffectValue`, `FSubstanceAmount`, `FIngredientInstance`, `FPlantPartHarvestData`, `FProcessedIngredient`, `FPotionResult`, `FTreatmentRequirement`, `FPatientBodyState`, `FPatientCondition`, `FTreatmentResult` — check this file before adding any new alchemy/disease-related struct

### Disease & Patient System
- `UDataAssetDisease` (`DataAssets/`) — disease tags, visible symptom tags, `TArray<FTreatmentRequirement>` (each requirement points at a required `UDataAssetAlchemyEfectDefinition` + value + mandatory flag), relevant nutrient tags, max allowed toxicity, progression speed
- `UPatientConditionComponent` (`Components/Disease/`) — attached to anything that can get sick/be treated (both `ANPCCharacter` and `AAlchemySimulatorCharacter` have one via a `Condition` member); holds `FPatientCondition` (disease, body state, severity, progress, visible symptoms); `ApplyPotion(FItemInstanceData)` is the entry point potions call into
- `UDiseaseTreatmentSubsystem` (`Subsystems/`, `UGameInstanceSubsystem`) — `EvaluateTreatment(FPotionResult, FPatientCondition)` matches a potion's effects against the disease's treatment requirements and returns an `FTreatmentResult` (cured / worsened / progress change / new symptoms)
- The player character itself is a valid potion target (has `Condition`), not just NPCs — self-treatment and NPC-treatment share the exact same pipeline

### NPCs & AI
- `ANPCCharacter` (`Characters/`) — `ACharacter` with `UInventoryComponent` + `UPatientConditionComponent` + a `bShouldGoToWork` schedule flag
- `AAlchemyNPCConroller` (`Controllers/`, name misspelled in-repo — keep the existing spelling for consistency) — `AAIController` owning a `UStateTreeAIComponent`; NPC behavior (schedules, being treated) runs through StateTree, not Blueprint AI or Behavior Trees
- `FSTT_UseItemTask` — see "Generic Use Item Pipeline" above; the StateTree-side hook for NPCs to consume/administer items

### Day/Night & Game Time
- `UGameTimeSubsystem` (`Subsystems/`, `UTickableWorldSubsystem`) — owns `FAlchemyGameTime` (Year/Month/Day/Hour/Minute); `RealSecondsPerGameMinute` (default 0.1) controls simulation speed; broadcasts `OnGameMinuteChanged` / `OnGameHourChanged` / `OnGameDayChanged`; `GetNormalizedDay()` returns time-of-day as 0..1 for lighting/shader use
- `ADayNightCycleController` (`Actors/`) — binds to `UGameTimeSubsystem`'s tick events and rotates a Sun and Moon `ADirectionalLight` based on `SunriseHour`/`SunsetHour`; this is the only consumer of `GetNormalizedDay`/game-time ticks for lighting today — new systems that care about time-of-day (NPC schedules, plant growth, shop hours) should subscribe to the same subsystem rather than polling `ADayNightCycleController`

### Minigame System
- `UAlchemyMinigameWidget` (`Widgets/Minigames/`) — base widget; exposes `OnMinigameFinished` delegate
- `UAlchemyCutMinigameWidget` — 20-segment ring with 4 green zones; indicator moves at 0.08 s/segment tick
- `UMinigameManagerComponent` (`Components/Minigame/`) — owns the active minigame widget lifecycle (create, show, destroy)
- Minigame results (`FMinigameResult` — success/score/quality multiplier) feed into `UProcessingComponent::BuildProcessedInstance` as the `minigameQuality` input

### UI / Widget Stack
- `UBaseGameWidget` (`Widgets/Menu/`) — base for modal widgets; lifecycle: `OnOpened` / `OnClosed` / `CanClose`
- `UWidgetStackManager` (`Widgets/`) — modal stack (Z-order starts at 100); `PushWidget` / `PopWidget` / `CloseAll`; `CanClose` veto prevents stack pop
- `UTableWidget` / `UTableInventoryWidget` (`Widgets/Tables/`) — workbench UI
- `UInventoryWidget` / `UInventorySlotWidget` (`Widgets/Inventory/`) — character inventory UI
- `UCharacterScreenWidget` (`Widgets/Menu/`) — full character screen (binds a `UInventoryWidget`); opened via `AAlchemySimulatorPlayerController::CharacterScreenWidgetClass`, same modal stack as everything else

### Player Controller
- `AAlchemySimulatorPlayerController` — manages input mapping context, widget stack reference, drag mechanics (`UInvDragOperation`), the active tool, and now also the character screen (`CharacterScreenWidgetClass`)

### Input Mode Management (single source of truth)
`AAlchemySimulatorPlayerController::RefreshInputMode()` is the **only** place that may call `SetInputMode`. It *derives* the mode from current state instead of having callers push one, in strict priority order:
1. **Active minigame** (`UMinigameManagerComponent::GetActiveMinigameWidget()`) → `FInputModeUIOnly` focused on that widget — fully modal
2. **Top stack widget** (`UWidgetStackManager::GetTopWidget()`) → `FInputModeGameAndUI` focused on that widget — the most recently opened widget owns click priority
3. **At a station** (`Interacting == true`, no widget open) → `FInputModeGameAndUI` with no widget focus + `FSlateApplication::SetAllUserFocusToGameViewport()`, so world clicks reach table items
4. **Plain gameplay** → `FInputModeGameOnly`, cursor hidden

Rules when touching this area:
- **Never call `SetInputMode`, `bShowMouseCursor`, `bEnableClickEvents`, or `SetIgnore*Input` directly** — change the underlying state, then call `RefreshInputMode()`
- Widget-stack changes refresh automatically: `BeginPlay` binds `HandleWidgetStackChanged` to `UWidgetStackManager::OnWidgetPushed` / `OnWidgetPopped`. `CloseAll` broadcasts once per popped widget, so every stack path is covered — this is why `PushWidget`/`PopWidget` no longer contain input-mode code
- `RemoveStationController` clears `Interacting = false` itself, *before* refreshing — callers used to set it afterwards, which made the refresh see a stale station state
- `RefreshInputMode()` must stay **idempotent**: `SetIgnoreLookInput`/`SetIgnoreMoveInput` are counter-based in UE, so it calls `ResetIgnore*Input()` first. Without that, `CloseAll` firing N refreshes would permanently freeze the pawn
- Always set `SetHideCursorDuringCapture(false)` on `FInputModeGameAndUI`. It defaults to `true`, which hides and re-centers the OS cursor for the whole left-click-drag gesture and freezes `DeprojectMousePositionToWorld` — this silently breaks world dragging
- Do **not** use `EMouseCaptureMode::NoCapture` to work around cursor issues; it disables capture-based click routing and makes the first click on the viewport an OS focus-activation click (a spurious "double-click required" bug)

## Key Conventions

- New non-cross-cutting classes go in a domain folder (`Actors/<X>/`, `Components/<X>/`, `Widgets/<X>/`, `DataAssets/`, `ItemDefinitions/`, `Subsystems/`, `Controllers/`, `Characters/`, `StateTree/Tasks/`) with `.h`/`.cpp` side by side — do not add new files to the old flat `Public/`/`Private/` unless the class is a dependency-free interface or shared struct file used module-wide
- BlueprintNativeEvent is preferred for interaction/use/AI callbacks so Blueprint subclasses can override without breaking C++ defaults (`IInteractable`, `IUsebale`, `UItemDefinitionBase::CanUseItem`/`UseItem`)
- New interactable world objects should implement `IInteractable` and register with `UInteractionDetectorComponent` via overlap
- Item definitions are DataAssets; create them in the editor and reference via soft pointers where load timing matters
- Alchemy/disease shared structs go in `Public/DataStructHelpers.h`; item-instance/use-pipeline structs go in `Public/ItemMetadata.h` — keep that split when adding new structs
- Anything that can be sick or treated gets a `UPatientConditionComponent`; anything a potion can act on is reached only through `FItemUseContext::TargetActor` + that component — don't special-case player vs. NPC treatment paths
- Variant systems (Combat/Platforming/SideScrolling) are self-contained — do not reference main alchemy classes from them
- Input mode is changed **only** via `AAlchemySimulatorPlayerController::RefreshInputMode()` — see "Input Mode Management" above
- **Changing `PrimaryActorTick.bCanEverTick` in a C++ constructor does not reach Blueprint subclasses that were already saved** — the old value is serialized into the BP's CDO and keeps overriding the new default. Symptom: `Tick` silently never runs on the BP instance while the C++ class looks correct. `APlantPart::BeginPlay` works around this by force-registering its tick function; do the same when enabling tick on a class that already has BP children
