// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HitableActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHitActor, AActor*, Perpetrator);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE5_BASICS_API UHitableActor : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHitableActor();
	UPROPERTY(BlueprintAssignable)
	FHitActor HitActorEvent;
	
	UFUNCTION(BlueprintCallable)
	void Hit(AActor* Perpetrator);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool CanBeHit = true;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
