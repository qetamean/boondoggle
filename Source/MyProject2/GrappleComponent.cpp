// Copyright Epic Games, Inc. All Rights Reserved.

#include "GrappleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Variant_Combat/Interfaces/CombatDamageable.h"
#include "Components/SkeletalMeshComponent.h"

UGrappleComponent::UGrappleComponent()
{
	// Tick is disabled by default and only enabled while an environmental grapple is active
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UGrappleComponent::BeginPlay()
{
	Super::BeginPlay();

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;

	// Create the cable and attach it to the character's mesh at the wrist socket
	GrappleCable = NewObject<UCableComponent>(GetOwner(), TEXT("GrappleCable"));
	GrappleCable->RegisterComponent();
	GrappleCable->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, CableAttachSocket);

	// The far end of the cable is driven manually via EndLocation each tick
	GrappleCable->bAttachEnd = false;
	GrappleCable->NumSegments = CableSegments;
	GrappleCable->CableWidth = CableWidth;
	GrappleCable->SetVisibility(false);
}

void UGrappleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GrappleState == EGrappleState::PullingToEnvironment)
	{
		TickPullToEnvironment(DeltaTime);
	}

	UpdateCable();
}

void UGrappleComponent::FireGrapple()
{
	// Don't fire if already grappling
	if (GrappleState != EGrappleState::Idle) return;

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;
	OwnerCharacter = Character;

	// Trace from the player's camera view so it matches the crosshair
	AController* Controller = Character->GetController();
	if (!Controller) return;

	FVector TraceStart;
	FRotator TraceRotation;
	Controller->GetPlayerViewPoint(TraceStart, TraceRotation);

	const FVector TraceEnd = TraceStart + TraceRotation.Vector() * GrappleRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);
	if (!bHit || !Hit.GetActor()) return;

	// If we hit something damageable, treat it as an enemy grapple (instant, no pull state)
	if (Hit.GetActor()->GetClass()->ImplementsInterface(UCombatDamageable::StaticClass()))
	{
		GrappleEnemy(Hit.GetActor(), Hit.ImpactPoint);
	}
	else
	{
		GrappleEnvironment(Hit.ImpactPoint);
	}
}

void UGrappleComponent::ReleaseGrapple()
{
	if (GrappleState == EGrappleState::Idle) return;

	GrappleState = EGrappleState::Idle;
	SetComponentTickEnabled(false);

	if (OwnerCharacter.IsValid())
	{
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	}

	// Hide the cable
	if (GrappleCable)
	{
		GrappleCable->SetVisibility(false);
	}
}

void UGrappleComponent::GrappleEnvironment(const FVector& HitPoint)
{
	GrapplePoint = HitPoint;
	GrappleState = EGrappleState::PullingToEnvironment;
	SetComponentTickEnabled(true);

	if (OwnerCharacter.IsValid())
	{
		// Switch to Flying so gravity doesn't fight the pull
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	}

	// Show the cable
	if (GrappleCable)
	{
		GrappleCable->SetVisibility(true);
	}
}

void UGrappleComponent::GrappleEnemy(AActor* Enemy, const FVector& HitPoint)
{
	if (!OwnerCharacter.IsValid()) return;

	const FVector OwnerLocation = OwnerCharacter->GetActorLocation();

	// Deal damage via the existing ICombatDamageable interface
	ICombatDamageable* Damageable = Cast<ICombatDamageable>(Enemy);
	if (Damageable)
	{
		// Impulse points from the impact toward the player (for hit reaction)
		FVector ImpulseDir = (OwnerLocation - HitPoint).GetSafeNormal();
		const FVector DamageImpulse = ImpulseDir * EnemyPullImpulse + FVector(0.f, 0.f, EnemyPullLiftImpulse);

		Damageable->ApplyDamage(GrappleHitDamage, GetOwner(), HitPoint, DamageImpulse);
	}

	// Also physically launch the enemy character toward the player
	ACharacter* EnemyCharacter = Cast<ACharacter>(Enemy);
	if (EnemyCharacter)
	{
		const FVector PullDir = (OwnerLocation - EnemyCharacter->GetActorLocation()).GetSafeNormal();
		const FVector PullImpulse = PullDir * EnemyPullImpulse + FVector(0.f, 0.f, EnemyPullLiftImpulse);
		EnemyCharacter->LaunchCharacter(PullImpulse, true, true);
	}

	// Enemy grapple is instant — stay in Idle so the player can fire again immediately
}

void UGrappleComponent::UpdateCable()
{
	if (!GrappleCable || GrappleState != EGrappleState::PullingToEnvironment) return;

	// Express the grapple anchor in the cable's local space so it tracks correctly
	const FTransform CableWorldTransform = GrappleCable->GetComponentTransform();
	GrappleCable->EndLocation = CableWorldTransform.InverseTransformPosition(GrapplePoint);
	GrappleCable->CableLength = (GrapplePoint - GrappleCable->GetComponentLocation()).Size();
}

void UGrappleComponent::TickPullToEnvironment(float DeltaTime)
{
	if (!OwnerCharacter.IsValid())
	{
		ReleaseGrapple();
		return;
	}

	const FVector CurrentLocation = OwnerCharacter->GetActorLocation();
	const FVector ToTarget = GrapplePoint - CurrentLocation;
	const float Distance = ToTarget.Size();

	if (Distance <= ArrivalDistance)
	{
		// Arrived — give the player an upward boost so they can jump off
		ReleaseGrapple();
		if (OwnerCharacter.IsValid())
		{
			OwnerCharacter->LaunchCharacter(FVector(0.f, 0.f, ArrivalLaunchBoost), false, true);
		}
		return;
	}

	// Drive velocity directly toward the anchor each frame
	OwnerCharacter->GetCharacterMovement()->Velocity = ToTarget.GetSafeNormal() * PullSpeed;
}
