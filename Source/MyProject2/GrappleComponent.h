// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrappleComponent.generated.h"

class UCableComponent;

/**
 *  Grapple device component — attach to any character to give them a grapple arm.
 *
 *  Two modes depending on what the line trace hits:
 *
 *  Environment grapple:
 *    Fires a hook toward the surface the player is aiming at, then pulls the
 *    character smoothly toward that point using Flying movement mode.
 *    Releases automatically on arrival or when ReleaseGrapple() is called.
 *
 *  Enemy grapple:
 *    Fires at any actor implementing ICombatDamageable. Deals instant damage
 *    and launches the target toward the player. Instant — no pull state.
 */
UENUM(BlueprintType)
enum class EGrappleState : uint8
{
	Idle,
	PullingToEnvironment,
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UGrappleComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UGrappleComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Fires the grapple in the direction the camera is looking */
	UFUNCTION(BlueprintCallable, Category="Grapple")
	void FireGrapple();

	/** Releases an active environmental grapple */
	UFUNCTION(BlueprintCallable, Category="Grapple")
	void ReleaseGrapple();

	/** Returns true if an environmental grapple is currently active */
	UFUNCTION(BlueprintPure, Category="Grapple")
	bool IsGrappling() const { return GrappleState != EGrappleState::Idle; }

	/** Returns the current grapple anchor point in world space (valid only while grappling) */
	UFUNCTION(BlueprintPure, Category="Grapple")
	FVector GetGrapplePoint() const { return GrapplePoint; }

	/** Returns the current grapple state */
	UFUNCTION(BlueprintPure, Category="Grapple")
	EGrappleState GetGrappleState() const { return GrappleState; }

protected:

	/** Maximum range of the grapple line trace */
	UPROPERTY(EditAnywhere, Category="Grapple", meta=(ClampMin=0, ClampMax=5000, Units="cm"))
	float GrappleRange = 2000.0f;

	/** Speed at which the player is pulled toward the grapple anchor */
	UPROPERTY(EditAnywhere, Category="Grapple|Environment", meta=(ClampMin=0, ClampMax=5000, Units="cm/s"))
	float PullSpeed = 1500.0f;

	/** Distance from the anchor at which the pull is considered complete */
	UPROPERTY(EditAnywhere, Category="Grapple|Environment", meta=(ClampMin=0, ClampMax=500, Units="cm"))
	float ArrivalDistance = 100.0f;

	/** Upward boost applied to the character's velocity when the pull completes */
	UPROPERTY(EditAnywhere, Category="Grapple|Environment", meta=(ClampMin=0, ClampMax=2000, Units="cm/s"))
	float ArrivalLaunchBoost = 400.0f;

	/** Name of the skeletal mesh socket the cable originates from (e.g. the wrist/hand bone) */
	UPROPERTY(EditAnywhere, Category="Grapple|Cable")
	FName CableAttachSocket = FName("hand_r");

	/** Number of segments in the cable — higher looks smoother but costs more */
	UPROPERTY(EditAnywhere, Category="Grapple|Cable", meta=(ClampMin=2, ClampMax=30))
	int32 CableSegments = 12;

	/** Visual width of the cable */
	UPROPERTY(EditAnywhere, Category="Grapple|Cable", meta=(ClampMin=0.1f, ClampMax=10.0f, Units="cm"))
	float CableWidth = 1.5f;

	/** Damage dealt to an enemy when the grapple hook hits them */
	UPROPERTY(EditAnywhere, Category="Grapple|Combat", meta=(ClampMin=0, ClampMax=100))
	float GrappleHitDamage = 1.5f;

	/** Horizontal impulse used to pull an enemy toward the player */
	UPROPERTY(EditAnywhere, Category="Grapple|Combat", meta=(ClampMin=0, ClampMax=5000, Units="cm/s"))
	float EnemyPullImpulse = 1000.0f;

	/** Upward impulse added when pulling an enemy (lifts them off the ground) */
	UPROPERTY(EditAnywhere, Category="Grapple|Combat", meta=(ClampMin=0, ClampMax=2000, Units="cm/s"))
	float EnemyPullLiftImpulse = 300.0f;

private:

	EGrappleState GrappleState = EGrappleState::Idle;
	FVector GrapplePoint = FVector::ZeroVector;
	TWeakObjectPtr<ACharacter> OwnerCharacter;

	/** Cable component created at runtime and attached to the owner's mesh */
	UPROPERTY()
	TObjectPtr<UCableComponent> GrappleCable;

	void GrappleEnvironment(const FVector& HitPoint);
	void GrappleEnemy(AActor* Enemy, const FVector& HitPoint);
	void TickPullToEnvironment(float DeltaTime);
	void UpdateCable();
};
