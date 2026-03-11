# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**MyProject2** is an Unreal Engine 5.7 Third Person C++ project with three self-contained gameplay variants. Each variant has its own character, game mode, and player controller, all derived from a shared base in `Source/MyProject2/`.

## Build Commands

### From VS Code
- `Ctrl+Shift+B` — compile via Unreal Build Tool (generated task)

### From Command Line
```bash
# Build editor target
"C:/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" MyProject2Editor Win64 Development "C:/Users/parat/OneDrive/Documents/Unreal Projects/MyProject2/MyProject2.uproject" -WaitMutex -FromMsBuild
```

### Hot Reload
In the Unreal Editor, use **Live Coding** (bottom-right toolbar) or `Ctrl+Alt+F11` to recompile without restarting.

### Regenerate VS Code Project Files
After modifying `MyProject2.uproject` or `MyProject2.Build.cs`:
- Unreal Editor → **Tools → Generate Visual Studio Code Project**

## Architecture

### Base Layer (`Source/MyProject2/`)
The abstract base classes shared by all variants:

| File | Purpose |
|---|---|
| `MyProject2Character` | Abstract base character — third-person camera (`SpringArmComponent` + `CameraComponent`), Enhanced Input bindings (`Move`, `Look`, `Jump`), virtual `DoMove/DoLook/DoJumpStart/DoJumpEnd` for Blueprint/UI calls |
| `MyProject2GameMode` | Base game mode |
| `MyProject2PlayerController` | Base player controller |

### Variants (`Source/MyProject2/Variant_*/`)
Each variant is a complete, independent gameplay mode extending the base layer:

#### `Variant_Combat/`
- `CombatCharacter`, `CombatGameMode`, `CombatPlayerController`
- Subdirs: `AI/`, `Animation/`, `Gameplay/`, `Interfaces/`, `UI/`
- Uses **StateTree** + **GameplayStateTree** plugins for AI behavior

#### `Variant_Platforming/`
- `PlatformingCharacter`, `PlatformingGameMode`, `PlatformingPlayerController`
- Subdirs: `Animation/`

#### `Variant_SideScrolling/`
- `SideScrollingCharacter`, `SideScrollingGameMode`, `SideScrollingPlayerController`, `SideScrollingCameraManager`
- Subdirs: `AI/`, `Gameplay/`, `Interfaces/`, `UI/`

### Active Plugins
- **StateTree** + **GameplayStateTree** — AI behavior trees (used in Combat and SideScrolling variants)
- **ModelingToolsEditorMode** — in-editor mesh modeling
- **VisualStudioTools** — VS/VS Code integration

### Module Dependencies (`MyProject2.Build.cs`)
Key public modules: `EnhancedInput`, `AIModule`, `StateTreeModule`, `GameplayStateTreeModule`, `UMG`, `Slate`

## Key Patterns

- **Input** uses UE5 Enhanced Input system — input actions are data assets bound via `UInputMappingContext` in `SetupPlayerInputComponent()`
- **New C++ classes** must be added via **Unreal Editor → Tools → New C++ Class** to ensure UBT registration
- All variant include paths are registered in `Build.cs` — cross-variant includes are allowed
