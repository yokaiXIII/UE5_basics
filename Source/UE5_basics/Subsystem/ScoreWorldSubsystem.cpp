// Fill out your copyright notice in the Description page of Project Settings.


#include "ScoreWorldSubsystem.h"

#include <UE5_basics/TP_Character/PlayerCharacter.h>

void UScoreWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	CurrentMode = EBallMatchMode::OneVsOne;
	SetupDefaultTeams();
	/*
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
	*/
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

void UScoreWorldSubsystem::SetupDefaultTeams()
{
	Teams.Empty();
	TeamsCounter = 0;

	FTeam Team1;
	Team1.TeamName = TEXT("Team1");
	Team1.Score = 0;
	Team1.TeamColor = FLinearColor::Blue;

	FTeam Team2;
	Team2.TeamName = TEXT("Team2");
	Team2.Score = 0;
	Team2.TeamColor = FLinearColor::Red;

	FTeam Team3;
	Team3.TeamName = TEXT("Team3");
	Team3.Score = 0;
	Team3.TeamColor = FLinearColor::Green;

	FTeam Team4;
	Team4.TeamName = TEXT("Team4");
	Team4.Score = 0;
	Team4.TeamColor = FLinearColor::Yellow;

	Teams.Add(Team1);
	Teams.Add(Team2);
	Teams.Add(Team3);
	Teams.Add(Team4);
}

void UScoreWorldSubsystem::SetMatchMode(EBallMatchMode NewMode)
{
	CurrentMode = NewMode;
	TeamsCounter = 0;

	if (GEngine)
	{
		const FString ModeText = (CurrentMode == EBallMatchMode::OneVsOne)
			? TEXT("Modalita: 1 vs 1")
			: TEXT("Modalita: Tutti contro tutti (Team 1-4)");
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, ModeText);
	}
}

//Adding score to player 
/*
void UScoreWorldSubsystem::AddScore(APlayerCharacter* ScorerActor, int amount)
{
	if (!IsValid(ScorerActor)) return;
	FString TeamString = ScorerActor->GetPlayerTeam();

	for (auto team : Teams)
	{
		if (team.TeamName == TeamString)
		{
			team.Score += amount;
		}
	}
}
*/
void UScoreWorldSubsystem::AddScore(APlayerCharacter* ScorerActor, int amount)
{
	if (!IsValid(ScorerActor)) return;
	const FString TeamString = ScorerActor->GetPlayerTeam();

	for (FTeam& team : Teams)
	{
		if (team.TeamName == TeamString)
		{
			team.Score += amount;
			currentScore = team.Score;
			OnScoreChanged.Broadcast(team.Score);
			break;
		}
	}
}

/*
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
*/

void UScoreWorldSubsystem::RegisterNewPlayer(APlayerCharacter* Player)
{
	if (!IsValid(Player) || Teams.Num() == 0)
		return;

	const int32 MaxTeamsForMode = (CurrentMode == EBallMatchMode::OneVsOne)
		? FMath::Min(2, Teams.Num())
		: Teams.Num();

	if (TeamsCounter >= MaxTeamsForMode || TeamsCounter < 0)
	{
		TeamsCounter = 0;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			10.f,
			FColor::Red,
			FString::Printf(TEXT("TeamCounter = %d"), TeamsCounter)
		);
	}

	Player->SetPlayerTeam(Teams[TeamsCounter]);
	TeamsCounter++;
}

TArray<FTeam> UScoreWorldSubsystem::GetAllTeams()
{
	return Teams;
}
