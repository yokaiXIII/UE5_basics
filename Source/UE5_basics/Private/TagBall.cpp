#include "TagBall.h"

#include "TP_TestCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Other/HitCounter.h"

ATagBall::ATagBall()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	SetRootComponent(PickupSphere);

	PickupSphere->InitSphereRadius(75.0f);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PickupSphere->SetCollisionObjectType(ECC_WorldDynamic);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	PickupSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	PickupSphere->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupSphere->SetGenerateOverlapEvents(true);
	PickupSphere->SetNotifyRigidBodyCollision(true);
	PickupSphere->SetUseCCD(true);
	PickupSphere->SetSimulatePhysics(false);

	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	BallMesh->SetupAttachment(PickupSphere);
	BallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATagBall::BeginPlay()
{
	Super::BeginPlay();

	OnActorBeginOverlap.AddDynamic(this, &ATagBall::HandleBeginOverlap);
	PickupSphere->OnComponentHit.AddDynamic(this, &ATagBall::HandleHit);

	// Gestisce anche un player già sopra la palla all'avvio.
	PickupSphere->UpdateOverlaps();

	TArray<AActor*> OverlappingActors;
	PickupSphere->GetOverlappingActors(
		OverlappingActors, ATP_TestCharacter::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		if (IsValid(Holder))
		{
			break;
		}

		if (GiveTo(Cast<ATP_TestCharacter>(Actor)))
		{
			break;
		}
	}
}

void ATagBall::HandleBeginOverlap(
	AActor* OverlappedActor, AActor* OtherActor)
{
	if (bInFlight)
	{
		ATP_TestCharacter* HitCharacter = Cast<ATP_TestCharacter>(OtherActor);
		if (IsValid(HitCharacter) && HitCharacter != LastThrower)
		{
			HitCharacter->Stun();
			bInFlight = false;
			PickupSphere->SetPhysicsLinearVelocity(FVector::ZeroVector);
			PickupSphere->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		}

		return;
	}

	// Una palla già raccolta si può rubare solo sparando.
	if (!IsValid(Holder))
	{
		GiveTo(Cast<ATP_TestCharacter>(OtherActor));
	}
}

bool ATagBall::GiveTo(ATP_TestCharacter* NewHolder)
{
	if (!IsValid(NewHolder) || Holder == NewHolder ||
		IsValid(NewHolder->HeldBall) ||
		NewHolder->IsExhausted() ||
		!IsValid(NewHolder->HealthComponent) ||
		NewHolder->HealthComponent->GetCurrentHealth() <= 0.0f)
	{
		return false;
	}

	PickupSphere->SetSimulatePhysics(false);
	PickupSphere->SetPhysicsLinearVelocity(FVector::ZeroVector);
	PickupSphere->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	bInFlight = false;

	if (IsValid(Holder))
	{
		Holder->HeldBall = nullptr;
	}

	Holder = NewHolder;
	Holder->HeldBall = this;

	SetActorEnableCollision(false);

	AttachToComponent(
		Holder->GetRootComponent(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	SetActorRelativeLocation(CarryOffset);
	SetActorRelativeRotation(FRotator::ZeroRotator);

	return true;
}

void ATagBall::Drop()
{
	Release(false);
}

void ATagBall::Release(bool bLaunched)
{
	if (IsValid(Holder))
	{
		if (bLaunched)
		{
			LastThrower = Holder;
		}

		Holder->HeldBall = nullptr;
	}

	Holder = nullptr;
	bInFlight = bLaunched;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorEnableCollision(true);
	PickupSphere->SetPhysicsLinearVelocity(FVector::ZeroVector);
	PickupSphere->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	PickupSphere->SetSimulatePhysics(true);
}

void ATagBall::HandleHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (bInFlight)
	{
		if (AHitCounter* HitCounter = Cast<AHitCounter>(OtherActor))
		{
			HitCounter->RegisterBallHit(this);
			return;
		}

		bInFlight = false;
		PickupSphere->SetPhysicsLinearVelocity(FVector::ZeroVector);
		PickupSphere->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
}

void ATagBall::Explode()
{
	if (IsValid(Holder))
	{
		Holder->HeldBall = nullptr;
	}

	Holder = nullptr;
	LastThrower = nullptr;
	bInFlight = false;
	Destroy();
}