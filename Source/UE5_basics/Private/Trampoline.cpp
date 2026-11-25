// Fill out your copyright notice in the Description page of Project Settings.


#include "Trampoline.h"

// Sets default values
ATrampoline::ATrampoline()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Collider = CreateDefaultSubobject<UBoxComponent>(FName("Collider"));
}

// Called when the game starts or when spawned
void ATrampoline::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATrampoline::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ATrampoline::AddForceToActor(AActor* TargetActor)
{
	if (!IsValid(TargetActor))
		return;

	UPrimitiveComponent* TargetComp = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent());
	TargetComp->AddForce(ForceDirection * JumpForce , EName::None);
	
}

