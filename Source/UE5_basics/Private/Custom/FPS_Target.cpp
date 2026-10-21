// Fill out your copyright notice in the Description page of Project Settings.


#include "Custom/FPS_Target.h"

// Sets default values
AFPS_Target::AFPS_Target()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AFPS_Target::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFPS_Target::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFPS_Target::Hit(APawn* EventOrigin)
{
	hitCounter++;
	HitEvent.Broadcast();
}

int AFPS_Target::GetHitCounter() const
{
	return hitCounter;
}
