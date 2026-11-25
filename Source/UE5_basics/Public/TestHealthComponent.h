// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TestHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHealtTestDelegate);
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE5_BASICS_API UTestHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTestHealthComponent();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float currentHealth = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float RecoveryAmount = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	bool eAutoRecovery = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float TimeToStartRecovery = 2.0f;
	
	
	UPROPERTY(BlueprintAssignable)
	FHealtTestDelegate OnHealthToZero;
	
	UPROPERTY(BlueprintAssignable)
	FHealtTestDelegate OnDamageTaken;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	float CurrentRecoveryTimer = 0.0f;
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	void GetDamage(int amount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ConsumeStamina(float amount);

	UFUNCTION(BlueprintPure, Category = "Health")
	bool HasStamina(float amount) const;
	
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const { return currentHealth; }
	
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	void recoverHealth(float amount);
		
};
