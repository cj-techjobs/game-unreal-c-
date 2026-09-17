// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMLightningProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "SOTMStunGameplayEffect.h"
#include "SOTMGameplay.h"

ASOTMLightningProjectile::ASOTMLightningProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(20.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->OnComponentHit.AddDynamic(this, &ASOTMLightningProjectile::HandleComponentHit);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASOTMLightningProjectile::HandleComponentBeginOverlap);
	SetRootComponent(CollisionComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(CollisionComponent);
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	// Without this, UProjectileMovementComponent teleports to each tick's
	// destination instead of sweeping -- overlaps only update at the
	// discrete post-move position, so a fast projectile can silently skip
	// past a target between ticks with no hit ever registering.
	ProjectileMovement->bSweepCollision = true;

	InitialLifeSpan = 10.0f;
}

void ASOTMLightningProjectile::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();
	ProjectileMovement->InitialSpeed = ProjectileSpeed;
	ProjectileMovement->MaxSpeed = ProjectileSpeed;
	ProjectileMovement->Velocity = GetActorForwardVector() * ProjectileSpeed;
}

void ASOTMLightningProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (FVector::DistSquared(GetActorLocation(), SpawnLocation) > FMath::Square(MaxRangeCm))
	{
		UE_LOG(LogSOTMGameplay, Verbose, TEXT("SOTMLightningProjectile: exceeded MaxRangeCm (%.0f) with no hit -- fizzled"), MaxRangeCm);
		Destroy();
	}
}

void ASOTMLightningProjectile::HandleComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == this || OtherActor == GetInstigator())
	{
		return;
	}

	ResolveImpact(Hit.ImpactPoint);
}

void ASOTMLightningProjectile::HandleComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetInstigator())
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (!TargetASC)
	{
		// Not a GAS actor (handbook Part 1 section 4's ISOTMDamageReceiver
		// carve-out is for environmental damage, not stun) -- an eligible
		// target for Lightning Throw's stun is exactly "has an ASC", which
		// in the current game means the player or an enemy; harmless
		// non-GAS clutter (props, foliage) is silently ignored rather than
		// stopping the bolt.
		return;
	}

	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
	UAbilitySystemComponent* SpecOwnerASC = SourceASC ? SourceASC : TargetASC;

	FGameplayEffectContextHandle EffectContext = SpecOwnerASC->MakeEffectContext();
	EffectContext.AddInstigator(GetInstigator() ? GetInstigator()->GetController() : nullptr, this);

	const FGameplayEffectSpecHandle SpecHandle = SpecOwnerASC->MakeOutgoingSpec(USOTMStunGameplayEffect::StaticClass(), 1.0f, EffectContext);
	if (SpecHandle.IsValid())
	{
		SpecOwnerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		UE_LOG(LogSOTMGameplay, Verbose, TEXT("SOTMLightningProjectile: applied stun to %s"), *GetNameSafe(OtherActor));
	}

	ResolveImpact(bFromSweep ? FVector(SweepResult.ImpactPoint) : OtherActor->GetActorLocation());
}

void ASOTMLightningProjectile::ResolveImpact(const FVector& ImpactLocation)
{
	if (ImpactEffectClass)
	{
		GetWorld()->SpawnActor<AActor>(ImpactEffectClass, ImpactLocation, GetActorRotation());
	}

	Destroy();
}
