// Fill out your copyright notice in the Description page of Project Settings.


#include "Custom/MyFirstActorHeal.h"
#include "UE5_basics/Custom/CustomHealthComponent.h"

class UCustomHealthComponent;

void AMyFirstActorHeal::ActivateEffect()
{
	auto beforeOverlapping = EnteredCharacters;
	Super::ActivateEffect();
	for (AActor* currentActor : EnteredCharacters)
	{
		if (!IsValid(currentActor) || beforeOverlapping.Contains(currentActor))
			continue;

		UCustomHealthComponent* healthComponent = currentActor->FindComponentByClass<UCustomHealthComponent>();
		if (IsValid(healthComponent) && healthComponent != nullptr)
		{
			healthComponent->GetHealing(HealAmount);
		}
	}
}
