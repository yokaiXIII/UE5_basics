// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomHealthComponent.h"

// Sets default values for this component's properties
UCustomHealthComponent::UCustomHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void UCustomHealthComponent::GetDamaged(int Amount)
{
	if (Amount == 0) return;

	auto AbsDamage = abs(Amount);
	CurrentHealth = std::clamp(CurrentHealth - AbsDamage, 0, MaxHealth);
	Damaged.Broadcast();
	TimeSinceLastDamaged = 0;

	if (CurrentHealth == 0)
		Died.Broadcast();
}

int UCustomHealthComponent::GetCurrentHealt()
{
	return CurrentHealth;
}

void UCustomHealthComponent::TriggerHealingTick(float DeltaTime)
{
	if (!AwaitTickHealingTimeCondition(DeltaTime) || !AwaitNextHealingTick(DeltaTime)) return;
	LastRegenTickTime = 0;

	GetHealing(RegenTickAmount);
}

void UCustomHealthComponent::GetHealing(int Amount)
{
	if (Amount == 0) return;

	auto AbsHeal = abs(Amount);
	CurrentHealth = std::clamp(CurrentHealth + AbsHeal, 0, MaxHealth);
	Healed.Broadcast();
}

// Called when the game starts
void UCustomHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	
}

bool UCustomHealthComponent::AwaitTickHealingTimeCondition(float DeltaTime)
{
	TimeSinceLastDamaged += DeltaTime;
	return CanRegenHealth && TimeSinceLastDamaged > TimeSinceLastHit;
}

inline bool UCustomHealthComponent::AwaitNextHealingTick(float DeltaTime)
{
	LastRegenTickTime += DeltaTime;
	return CanRegenHealth && LastRegenTickTime >= RegenTickTime;
}

// Called every frame
void UCustomHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

