// Fill out your copyright notice in the Description page of Project Settings.

#include "SoccerSubsystem.h"
#include "TagBall.h"
#include "TP_TestCharacter.h"
#include "Other/HitCounter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void USoccerSubsystem::InitializeMatch()
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	for (const TWeakObjectPtr<AHitCounter>& GoalPtr : RegisteredGoals)
	{
		if (AHitCounter* Goal = GoalPtr.Get())
		{
			Goal->SetGoalOwner(nullptr, FLinearColor::White);
		}
	}

	PlayerGoals.Reset();
	RegisteredPlayers.Reset();
	RegisteredGoals.Reset();

	// Raccoglie tutti i player e tutte le porte presenti, inclusi quelli già spawnati.
	TArray<AActor*> FoundPlayers;
	UGameplayStatics::GetAllActorsOfClass(World, ATP_TestCharacter::StaticClass(), FoundPlayers);

	TArray<AActor*> FoundGoals;
	UGameplayStatics::GetAllActorsOfClass(World, AHitCounter::StaticClass(), FoundGoals);

	for (AActor* Actor : FoundPlayers)
	{
		if (ATP_TestCharacter* Player = Cast<ATP_TestCharacter>(Actor))
		{
			RegisteredPlayers.Add(Player);
		}
	}

	for (AActor* Actor : FoundGoals)
	{
		if (AHitCounter* Goal = Cast<AHitCounter>(Actor))
		{
			RegisteredGoals.Add(Goal);
			Goal->SetGoalOwner(nullptr, FLinearColor::White);
		}
	}

	TryAssignGoals();
}

void USoccerSubsystem::RegisterPlayer(ATP_TestCharacter* Player)
{
	if (IsValid(Player))
	{
		RegisteredPlayers.AddUnique(TWeakObjectPtr<ATP_TestCharacter>(Player));
		TryAssignGoals();
	}
}

void USoccerSubsystem::UnregisterPlayer(ATP_TestCharacter* Player)
{
	if (Player == nullptr)
	{
		return;
	}

	const TWeakObjectPtr<ATP_TestCharacter> PlayerPtr(Player);
	if (TWeakObjectPtr<AHitCounter>* GoalPtr = PlayerGoals.Find(PlayerPtr))
	{
		if (AHitCounter* Goal = GoalPtr->Get())
		{
			Goal->SetGoalOwner(nullptr, FLinearColor::White);
		}
		PlayerGoals.Remove(PlayerPtr);
	}

	RegisteredPlayers.Remove(PlayerPtr);
	TryAssignGoals();
}

void USoccerSubsystem::RegisterGoal(AHitCounter* Goal)
{
	if (IsValid(Goal))
	{
		RegisteredGoals.AddUnique(TWeakObjectPtr<AHitCounter>(Goal));
		TryAssignGoals();
	}
}

void USoccerSubsystem::UnregisterGoal(AHitCounter* Goal)
{
	if (Goal == nullptr)
	{
		return;
	}

	const TWeakObjectPtr<AHitCounter> GoalPtr(Goal);
	RegisteredGoals.Remove(GoalPtr);
	for (auto It = PlayerGoals.CreateIterator(); It; ++It)
	{
		if (It.Value() == GoalPtr)
		{
			It.RemoveCurrent();
		}
	}

	TryAssignGoals();
}

AHitCounter* USoccerSubsystem::GetGoalForPlayer(ATP_TestCharacter* Player) const
{
	if (const TWeakObjectPtr<AHitCounter>* GoalPtr = PlayerGoals.Find(TWeakObjectPtr<ATP_TestCharacter>(Player)))
	{
		return GoalPtr->Get();
	}

	return nullptr;
}

void USoccerSubsystem::TryAssignGoals()
{
	RegisteredPlayers.RemoveAll([](const TWeakObjectPtr<ATP_TestCharacter>& Player)
	{
		return !Player.IsValid();
	});
	RegisteredGoals.RemoveAll([](const TWeakObjectPtr<AHitCounter>& Goal)
	{
		return !Goal.IsValid();
	});
	for (auto It = PlayerGoals.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid() || !It.Value().IsValid())
		{
			if (AHitCounter* Goal = It.Value().Get())
			{
				Goal->SetGoalOwner(nullptr, FLinearColor::White);
			}
			It.RemoveCurrent();
		}
	}

	int32 PlayerIndex = 0;
	for (const TWeakObjectPtr<ATP_TestCharacter>& PlayerPtr : RegisteredPlayers)
	{
		ATP_TestCharacter* Player = PlayerPtr.Get();
		if (!IsValid(Player))
		{
			continue;
		}

		const FLinearColor Color = TeamColors.IsValidIndex(PlayerIndex) ? TeamColors[PlayerIndex] : FLinearColor::White;
		Player->SetTeamColor(Color);
		++PlayerIndex;

		TWeakObjectPtr<AHitCounter>* AssignedGoalPtr = PlayerGoals.Find(PlayerPtr);
		if (AssignedGoalPtr && AssignedGoalPtr->IsValid())
		{
			AssignedGoalPtr->Get()->SetGoalOwner(Player, Color);
			continue;
		}

		AHitCounter* BestGoal = nullptr;
		float MinDistanceSq = TNumericLimits<float>::Max();
		for (const TWeakObjectPtr<AHitCounter>& GoalPtr : RegisteredGoals)
		{
			AHitCounter* Goal = GoalPtr.Get();
			if (!IsValid(Goal))
			{
				continue;
			}

			bool bAlreadyAssigned = false;
			for (const TPair<TWeakObjectPtr<ATP_TestCharacter>, TWeakObjectPtr<AHitCounter>>& Pair : PlayerGoals)
			{
				if (Pair.Value == GoalPtr)
				{
					bAlreadyAssigned = true;
					break;
				}
			}

			if (bAlreadyAssigned)
			{
				continue;
			}

			const float DistanceSq = FVector::DistSquared(Player->GetActorLocation(), Goal->GetActorLocation());
			if (DistanceSq < MinDistanceSq)
			{
				MinDistanceSq = DistanceSq;
				BestGoal = Goal;
			}
		}

		if (IsValid(BestGoal))
		{
			PlayerGoals.Add(PlayerPtr, BestGoal);
			BestGoal->SetGoalOwner(Player, Color);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SoccerSubsystem: Nessuna porta libera disponibile per %s."), *Player->GetName());
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					4.0f,
					FColor::Yellow,
					FString::Printf(TEXT("Nessuna porta libera per %s."), *Player->GetName()));
			}
		}
	}
}

void USoccerSubsystem::SetBallSpawnPointActor(AActor* SpawnActor)
{
	BallSpawnPointActor = SpawnActor;
}

void USoccerSubsystem::RegisterGoalFromBall(ATagBall* Ball, AHitCounter* GoalHit, int32 Points)
{
	if (!IsValid(Ball) || !Ball->IsInFlight())
	{
		return;
	}

	// Cattura automaticamente la classe effettiva della palla se non configurata manualmente
	if (!IsValid(BallClass))
	{
		BallClass = Ball->GetClass();
	}

	ATP_TestCharacter* Thrower = Ball->GetLastThrower();
	ATP_TestCharacter* GoalDefender = IsValid(GoalHit) ? GoalHit->GetGoalOwner() : nullptr;
	if (!IsValid(Thrower) || !IsValid(GoalDefender))
	{
		return;
	}

	ATP_TestCharacter* AwardedPlayer = Thrower == GoalDefender ? FindOpponent(Thrower) : Thrower;
	if (!IsValid(AwardedPlayer))
	{
		return;
	}

	RegisterPlayerGoal(AwardedPlayer, Points);
	if (AHitCounter* AwardedPlayerGoal = GetGoalForPlayer(AwardedPlayer))
	{
		AwardedPlayerGoal->AddScore(Points);
	}

	// Distrugge la palla attuale e pianifica il respawn
	Ball->Explode();
	ScheduleBallRespawn();
}

ATP_TestCharacter* USoccerSubsystem::FindOpponent(ATP_TestCharacter* Player) const
{
	for (const TWeakObjectPtr<ATP_TestCharacter>& PlayerPtr : RegisteredPlayers)
	{
		ATP_TestCharacter* Candidate = PlayerPtr.Get();
		if (IsValid(Candidate) && Candidate != Player)
		{
			return Candidate;
		}
	}

	return nullptr;
}

void USoccerSubsystem::RegisterPlayerGoal(ATP_TestCharacter* Scorer, int32 Points)
{
	++TotalGoalsCount;
	OnTotalGoalsChanged.Broadcast(TotalGoalsCount);

	int32 NewScore = Points;

	if (IsValid(Scorer))
	{
		if (int32* CurrentScore = PlayerScores.Find(Scorer))
		{
			*CurrentScore += Points;
			NewScore = *CurrentScore;
		}
		else
		{
			PlayerScores.Add(Scorer, Points);
		}

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.5f,
				FColor::Green,
				FString::Printf(TEXT("GOAL! Player: %s | Punteggio: %d | Goal Totali: %d"),
					*Scorer->GetName(), NewScore, TotalGoalsCount));
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.5f,
				FColor::Yellow,
				FString::Printf(TEXT("GOAL! Autore sconosciuto | Goal Totali: %d"), TotalGoalsCount));
		}
	}

	OnGoalScored.Broadcast(Scorer, NewScore);
}

void USoccerSubsystem::ScheduleBallRespawn()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RespawnTimerHandle,
			this,
			&USoccerSubsystem::SpawnNewBall,
			RespawnDelay,
			false);
	}
}

void USoccerSubsystem::SpawnNewBall()
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	TSubclassOf<ATagBall> ClassToSpawn = IsValid(BallClass) ? BallClass : TSubclassOf<ATagBall>(ATagBall::StaticClass());
	if (!IsValid(ClassToSpawn))
	{
		return;
	}

	FTransform SpawnTransform = FTransform::Identity;
	if (BallSpawnPointActor.IsValid())
	{
		SpawnTransform = BallSpawnPointActor->GetActorTransform();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATagBall* NewBall = World->SpawnActor<ATagBall>(ClassToSpawn, SpawnTransform, SpawnParams);
	if (IsValid(NewBall))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Cyan,
				TEXT("Palla rigenerata allo Spawn Point!"));
		}
	}
}

int32 USoccerSubsystem::GetScoreForPlayer(const ATP_TestCharacter* Player) const
{
	if (!IsValid(Player))
	{
		return 0;
	}

	if (const int32* FoundScore = PlayerScores.Find(Player))
	{
		return *FoundScore;
	}

	return 0;
}

void USoccerSubsystem::ResetMatchStats()
{
	PlayerScores.Empty();
	TotalGoalsCount = 0;
	OnTotalGoalsChanged.Broadcast(TotalGoalsCount);
}

