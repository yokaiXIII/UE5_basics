// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CustomHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDamageTakenDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHealedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDeadDelegate);



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE5_BASICS_API UCustomHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCustomHealthComponent();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxHealth;

	UFUNCTION(BlueprintCallable)
	void GetDamaged(int Amount);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetCurrentHealt();

	UPROPERTY(BlueprintAssignable)
	FDamageTakenDelegate Damaged;
	UPROPERTY(BlueprintAssignable)
	FDeadDelegate Died;

	UFUNCTION(BlueprintImplementableEvent)
	bool IsAlive();

	// Healing part
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool CanRegenHealth = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeSinceLastHit = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RegenTickTime = .25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int RegenTickAmount = 1;

	UFUNCTION(BlueprintCallable)
	void TriggerHealingTick(float DeltaTime);

	UPROPERTY(BlueprintAssignable)
	FHealedDelegate Healed;
	
	UFUNCTION(BlueprintCallable)
	void GetHealing(int Amount);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	float TimeSinceLastDamaged;
	float LastRegenTickTime = 0;
	bool AwaitTickHealingTimeCondition(float DeltaTime);
	bool AwaitNextHealingTick(float DeltaTime);

private:
	int CurrentHealth;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
