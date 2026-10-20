// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UE5_basics/Basics/MyFirstActor.h"
#include "MyFirstActorDamage.generated.h"

/**
 * 
 */
UCLASS()
class UE5_BASICS_API AMyFirstActorDamage : public AMyFirstActor
{
	GENERATED_BODY()

	protected:
	virtual void ActivateEffect() override;

	public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int DamageDealt;
};
