// Fill out your copyright notice in the Description page of Project Settings.


#include "Bucket.h"

// Sets default values
ABucket::ABucket()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABucket::BeginPlay()
{
	Super::BeginPlay();

	RespawnRandom();
}

// Called every frame
void ABucket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void ABucket::RespawnRandom()
{
	if (BallRespawnPoints.Num() == 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Attenzione: Nessun punto di respawn impostato per la palla!"));
		return;
	}

	// Sceglie un indice casuale tra 0 e il numero di punti - 1
	int32 RandomIndex = FMath::RandRange(0, BallRespawnPoints.Num() - 1);

	// Prende il transform corrispondente
	FTransform TargetTransform = BallRespawnPoints[RandomIndex];

	// Sposta la palla nel punto scelto
	SetActorTransform(TargetTransform, false, nullptr, ETeleportType::TeleportPhysics);

	// Azzera la fisica se la root è un componente primitivo
	if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		PrimComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
		PrimComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
}
