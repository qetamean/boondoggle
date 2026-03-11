// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatHUD.generated.h"

/**
 *  Screen-space HUD widget for the local player.
 *
 *  The C++ side calls into Blueprint via implementable events so the visual
 *  design (bars, animations, icons) lives entirely in the Widget Blueprint.
 *
 *  Usage:
 *   - Subclass this in Blueprint and implement the events below.
 *   - Assign the subclass to HUDWidgetClass on CombatPlayerController.
 */
UCLASS(abstract)
class UCombatHUD : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Called whenever the player's HP changes — update bars, numbers, etc. */
	UFUNCTION(BlueprintImplementableEvent, Category="HUD|Health")
	void UpdateHealth(float CurrentHP, float MaxHP);

	/** Called when the player takes damage — use for screen vignette / red flash */
	UFUNCTION(BlueprintImplementableEvent, Category="HUD|Health")
	void ShowDamageFlash();

	/** Called when the player receives healing — use for green flash or glow */
	UFUNCTION(BlueprintImplementableEvent, Category="HUD|Health")
	void ShowHealEffect();

	/** Called when the player dies — use to fade to black or show death screen */
	UFUNCTION(BlueprintImplementableEvent, Category="HUD|Health")
	void ShowDeathScreen();

	/** Called when the player respawns — use to fade back in */
	UFUNCTION(BlueprintImplementableEvent, Category="HUD|Health")
	void HideDeathScreen();
};
