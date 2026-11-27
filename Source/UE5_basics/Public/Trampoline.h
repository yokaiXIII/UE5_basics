// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Trampoline.generated.h"

UCLASS()
class UE5_BASICS_API ATrampoline : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATrampoline();

	UPROPERTY(EditAnywhere , BlueprintReadWrite)
	float JumpForce;

	UPROPERTY(EditAnywhere , BlueprintReadWrite)
	FVector ForceDirection;

	UPROPERTY(EditAnywhere , BlueprintReadWrite)
	UBoxComponent* Collider;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void AddForceToActor(AActor* TargetActor);

};
