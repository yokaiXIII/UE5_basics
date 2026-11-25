// Fill out your copyright notice in the Description page of Project Settings.


#include "ScoreWorldSubsystem.h"

#include <UE5_basics/TP_Character/PlayerCharacter.h>

void UScoreWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	TeamsCounter = 0;
	FTeam* Team1 = new FTeam();
	Team1->Score = 0;
	Team1->TeamName = "Team1";
	Team1->TeamColor = FLinearColor::Blue;
	
	FTeam* Team2 = new FTeam();
	Team2->Score = 0;
	Team2->TeamName = "Team2";
	Team2->TeamColor = FLinearColor::Red;
	
	Teams.Add(*Team1);
	Teams.Add(*Team2);
	/*
	const UEnum* Enum = StaticEnum<ETeam>();
	for (int32 i = 0; i < Enum->NumEnums() - 1; ++i)
	{
		if (Enum->HasMetaData(TEXT("Hidden"), i))
			continue;   // salta le voci nascoste come MAX
		
		ETeam Type = static_cast<ETeam>(Enum->GetValueByIndex(i));
		scoreMap.Add(Type, currentScore);
	}
	*/	
	
	// insert inizialize logic
}

void UScoreWorldSubsystem::Deinitialize()
{
	//insert cleaning logic 
	Super::Deinitialize();
}
//Adding score to player 
void UScoreWorldSubsystem::AddScore(APlayerCharacter* ScorerActor, int amount)
{

}
//assign player to a team
void UScoreWorldSubsystem::RegisterNewPlayer(APlayerCharacter* Player)
{
	if (!IsValid(Player))
		return;
	if (TeamsCounter > Teams.Num() - 1)
		TeamsCounter = 0;
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, "TeamCounter = " + (TeamsCounter));

	Player->SetPlayerTeam(Teams[TeamsCounter]);
	TeamsCounter++;
}

TArray<FTeam> UScoreWorldSubsystem::GetAllTeams()
{
	return Teams;	
}
