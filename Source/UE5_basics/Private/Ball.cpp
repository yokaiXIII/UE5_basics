// Fill out your copyright notice in the Description page of Project Settings.

#include "UE5_basics/Public/Ball.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "UE5_basics/Other/HealthComponent.h"

// Sets default values
ABall::ABall()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Root: corpo fisico
	Collider = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collider"));
	SetRootComponent(Collider);
	Collider->InitSphereRadius(50.f);
	Collider->SetCollisionProfileName(TEXT("PhysicsActor"));
	Collider->BodyInstance.bUseCCD = true;
	Collider->SetSimulatePhysics(true);
	Collider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// Trigger di raccolta
	PickupTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("Pickup Trigger"));
	PickupTrigger->SetupAttachment(Collider);
	PickupTrigger->InitSphereRadius(60.f);
	PickupTrigger->SetCollisionProfileName(TEXT("Trigger"));
	PickupTrigger->SetGenerateOverlapEvents(true);
}

void ABall::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Cerca la StaticMesh aggiunta nel Blueprint come figlia del Collider
	TArray<USceneComponent*> ColliderChildren;
	Collider->GetChildrenComponents(false, ColliderChildren);

	for (USceneComponent* Child : ColliderChildren)
	{
		if (UStaticMeshComponent* SM = Cast<UStaticMeshComponent>(Child))
		{
			Mesh = SM;
			break;
		}
	}

	if (ensureMsgf(Mesh, TEXT("%s: nessuna StaticMesh figlia del Collider"), *GetName()))
	{
		// La mesh è solo visiva: fisica e collisione le gestisce il Collider
		Mesh->SetSimulatePhysics(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

// Called when the game starts or when spawned
void ABall::BeginPlay()
{
	Super::BeginPlay();

	// PickupTrigger->OnComponentBeginOverlap.AddDynamic(this, &ABall::OnPickupOverlap);
}

// Called every frame
void ABall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// ---------------------------------------------------------------------------
// Raccolta / rilascio
// ---------------------------------------------------------------------------

void ABall::OnPickupOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                            bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (!Player) return;

	// Appena lanciata non la riprende chi l'ha tirata
	if (Player == Holder && GetWorldTimerManager().IsTimerActive(ColliderHolderResetHandle))
		return;

	RetrivedBy(Player);
}

void ABall::SetHolderIgnoresBall(bool bIgnore)
{
	if (!IsValid(Holder)) return;

	UCapsuleComponent* Capsule = Holder->GetCapsuleComponent();
	USkeletalMeshComponent* CharMesh = Holder->GetMesh();

	if (bIgnore)
	{
		// Salva le response originali per ripristinarle dopo
		SavedCapsuleResponse = Capsule->GetCollisionResponseToChannel(ECC_PhysicsBody);
		SavedMeshResponse = CharMesh->GetCollisionResponseToChannel(ECC_PhysicsBody);

		Capsule->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
		CharMesh->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
	}
	else
	{
		Capsule->SetCollisionResponseToChannel(ECC_PhysicsBody, SavedCapsuleResponse);
		CharMesh->SetCollisionResponseToChannel(ECC_PhysicsBody, SavedMeshResponse);
	}

	// Movimento del personaggio (sweep della capsula)
	Capsule->IgnoreActorWhenMoving(this, bIgnore);

}

void ABall::SetBallHolder(APlayerCharacter* NewHolder)
{
	if (!IsValid(NewHolder))
		return;
	if (IsValid(Holder))
	{
		auto OldHealthComponent = Holder->HealthComponent;
		if (IsValid(OldHealthComponent))
			OldHealthComponent->StopDamageOverTime();
	}
	Holder = NewHolder;
	auto HealthComponent = NewHolder->HealthComponent;
	if (IsValid(HealthComponent))
		HealthComponent->TriggerDamageOverTime(DamageFrequency, DamageAmount);
}

bool ABall::RetrivedBy(APlayerCharacter* perpetrator)
{
	if (!CanBePickedUpBy(perpetrator)) return false;

	if (GetWorldTimerManager().IsTimerActive(ColliderHolderResetHandle))
	{
		StopColliderHolderTimer();
		ColliderHolderTimerCallback();
	}

	SetBallHolder(perpetrator);

	Collider->SetSimulatePhysics(false);
	ToggleCollision(false);

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, TEXT("Retrived!!!"));
	
	return true;
}

void ABall::Released(FVector forward, bool resetHolder)
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	bClearHolderAfterTimer = resetHolder;
	SetColliderHolderTimer();

	// Il lanciatore ignora la palla PRIMA che la collisione si riattivi
	SetHolderIgnoresBall(true);

	if (IsValid(Holder))
	{
		auto OldHealthComponent = Holder->HealthComponent;
		if (IsValid(OldHealthComponent))
			OldHealthComponent->StopDamageOverTime();
		// setting new position for the ball
		const float Offset = Holder->GetCapsuleComponent()->GetScaledCapsuleRadius()
						   + Collider->GetScaledSphereRadius() + 10.f;

		const FVector Start = Holder->GetActorLocation()
							+ Holder->GetActorForwardVector() * Offset
							+ FVector(0.f, 0.f, 30.f);

		SetActorLocation(Start, false, nullptr, ETeleportType::TeleportPhysics);
	}

	Collider->SetCollisionProfileName(TEXT("PhysicsActor"));
	Collider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Collider->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	// niente più ECC_Pawn ignorato: gli altri giocatori vengono colpiti normalmente
	Collider->BodyInstance.bUseCCD = true;

	ToggleCollision(true);
	Collider->SetSimulatePhysics(true);
	Collider->SetPhysicsLinearVelocity(FVector::ZeroVector);
	Collider->AddImpulse(forward.GetSafeNormal() * ThrowStrength, NAME_None, true);

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("Released!!!"));
}

void ABall::ToggleCollision(bool ctx)
{
	SetActorEnableCollision(ctx);
}

// ---------------------------------------------------------------------------
// Timer per ignorare il player dopo il lancio
// ---------------------------------------------------------------------------

void ABall::SetColliderHolderTimer()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// SetTimer su un handle già attivo lo resetta da solo
	World->GetTimerManager().SetTimer(
		ColliderHolderResetHandle,
		this,
		&ABall::ColliderHolderTimerCallback,
		IgnoreHolderTime
	);
}

void ABall::StopColliderHolderTimer()
{
	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(ColliderHolderResetHandle);
}

void ABall::ColliderHolderTimerCallback()
{
	SetHolderIgnoresBall(false);   // il lanciatore torna a collidere con la palla

	if (bClearHolderAfterTimer)
	{
		Holder = nullptr;
		bClearHolderAfterTimer = false;
	}

}

bool ABall::CanBePickedUpBy(const APlayerCharacter* Player) const
{
	if (!IsValid(Player)) return false;
	if (bIsBeingReleased) return false;                       // in pieno lancio
	if (GetAttachParentActor() != nullptr) return false;      // già in mano a qualcuno
	if (Player == Holder && GetWorldTimerManager().IsTimerActive(ColliderHolderResetHandle))
		return false;                                         // appena lanciata da lui
	return true;
}
