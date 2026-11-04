// Fill out your copyright notice in the Description page of Project Settings.


#include "Custom/HitableActor.h"

// Sets default values for this component's properties
UHitableActor::UHitableActor()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void UHitableActor::Hit(AActor* Perpetrator)
{
	if (!CanBeHit) return;
	HitActorEvent.Broadcast(Perpetrator);
}

// Called when the game starts
void UHitableActor::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UHitableActor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

