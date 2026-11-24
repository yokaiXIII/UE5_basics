// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "UE5_basics/TP_Character/PlayerCharacter.h"
#include "Ball.generated.h"

UCLASS()
class UE5_BASICS_API ABall : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABall();

	UPROPERTY(EditAnywhere , BlueprintReadWrite)
	int BallDamageOverTime; // It's the damage that the player gets when he holds the ball

	UPROPERTY(EditAnywhere , BlueprintReadWrite)
	USphereComponent* Collider;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	
	UPROPERTY(VisibleAnywhere , Blueprintable)
	APlayerCharacter* Holder; // The player who threw and posses this ball
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
	void SetBallHolder(APlayerCharacter* NewHolder); 
	
	
	void RetrivedBy(APlayerCharacter* perpetrator);
	void Released(FVector forward, bool resetHolder = false);

	UFUNCTION(BlueprintCallable)
	void ToggleCollision(bool ctx);

};
