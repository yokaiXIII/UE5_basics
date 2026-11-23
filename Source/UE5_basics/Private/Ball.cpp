// Fill out your copyright notice in the Description page of Project Settings.


#include "UE5_basics/Public/Ball.h"

// Sets default values
ABall::ABall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Collider = CreateDefaultSubobject<USphereComponent>(FName("Sphere Collider"));
}

// Called when the game starts or when spawned
void ABall::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ABall::SetBallHolder(APlayerCharacter* NewHolder)
{
	if (IsValid(NewHolder))
		return;
	Holder = NewHolder;
}

void ABall::RetrivedBy(APlayerCharacter* perpetrator)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, "Retrived!!!");
}

void ABall::Released(FVector forward, bool resetHolder)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, "Released!!!");
}

