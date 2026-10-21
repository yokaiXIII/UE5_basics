// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPS_Target.h"
#include "GameFramework/Actor.h"
#include "Ball.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBallRetrived);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBallReleased);
UCLASS()
class UE5_BASICS_API ABall : public AFPS_Target
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABall();
	UPROPERTY(BlueprintAssignable)
	FBallRetrived RetrivedEvent;
	
	UPROPERTY(BlueprintAssignable)
	FBallReleased ReleasedEvent;

	UPROPERTY(BlueprintReadOnly)
	ACharacter* Holder;

	UPROPERTY(BlueprintReadOnly)
	TArray<UStaticMeshComponent*> Components;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFUNCTION(BlueprintCallable)
	void ChangeMeshVisibility(bool status);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable)
	void Taken(ACharacter* NewHolder);
	UFUNCTION(BlueprintCallable)
	ACharacter* Released();

	virtual void Hit(APawn* EventOrigin) override;
};
