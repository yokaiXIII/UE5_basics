// Fill out your copyright notice in the Description page of Project Settings.


#include "ScoreWorldSubsystem.h"

void UScoreWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	const UEnum* Enum = StaticEnum<ETeam>();
	for (int32 i = 0; i < Enum->NumEnums() - 1; ++i)
	{
		if (Enum->HasMetaData(TEXT("Hidden"), i))
			continue;   // salta le voci nascoste come MAX
		
		ETeam Type = static_cast<ETeam>(Enum->GetValueByIndex(i));
		scoreMap.Add(Type, currentScore);
	}
		
	
	// insert inizialize logic
}

void UScoreWorldSubsystem::Deinitialize()
{
	//insert cleaning logic 
	Super::Deinitialize();
}
//Adding score to player 
void UScoreWorldSubsystem::AddScore(AActor* ScorerActor, int amount)
{
	
}

