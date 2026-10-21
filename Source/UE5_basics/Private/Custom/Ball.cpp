// Fill out your copyright notice in the Description page of Project Settings.


#include "Custom/Ball.h"

#include "GameFramework/Character.h"

// Sets default values
ABall::ABall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABall::BeginPlay()
{
	Super::BeginPlay();

	GetComponents<UStaticMeshComponent>(Components);
}

void ABall::ChangeMeshVisibility(bool status)
{
	for (auto mesh : Components)
	{
		mesh->SetVisibility(status);
		mesh->SetCollisionEnabled(status ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	}
}

// Called every frame
void ABall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABall::Taken(ACharacter* NewHolder)
{
	Holder = NewHolder;
	ChangeMeshVisibility(false);
}

ACharacter* ABall::Released()
{
	auto OldHolder = Holder;
	Holder = nullptr;
	this->SetActorLocation(OldHolder->GetActorForwardVector() * 100);
	
	ChangeMeshVisibility(true);
	
	return OldHolder;
}

void ABall::Hit(APawn* EventOrigin)
{
	Super::Hit(EventOrigin);

	ACharacter* NewHolder = Cast<ACharacter>(EventOrigin);
    if (!IsValid(NewHolder)) return;
    Taken(NewHolder);
}

