// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPS_Target.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFPS_Target_Hit);
UCLASS()
class UE5_BASICS_API AFPS_Target : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPS_Target();
	
	UPROPERTY(BlueprintAssignable)
	FFPS_Target_Hit HitEvent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(BlueprintReadOnly)
	int hitCounter = 0;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable)
	virtual void Hit(APawn* EventOrigin);

	UFUNCTION(BlueprintCallable)
	int GetHitCounter() const;
};
