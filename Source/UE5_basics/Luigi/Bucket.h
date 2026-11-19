// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitComponent.h"
#include "GameFramework/Actor.h"
#include "Bucket.generated.h"

UCLASS()
class UE5_BASICS_API ABucket : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABucket();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Respawn")
	AActor* Ball;
	
	// Lista dei transform in cui la palla può respawnare
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	TArray<FTransform> BallRespawnPoints;

	// Funzione da chiamare via evento per il respawn casuale
	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void RespawnRandom(AActor* Striker);
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Respawn")
	UHitComponent* HitComponent;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
