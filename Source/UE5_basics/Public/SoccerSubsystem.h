// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SoccerSubsystem.generated.h"

class ATP_TestCharacter;
class ATagBall;
class AHitCounter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGoalScoredSignature, ATP_TestCharacter*, Scorer, int32, NewPlayerScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTotalGoalsChangedSignature, int32, TotalGoals);

/**
 * World subsystem that manages soccer match stats, goal tracking, and score allocation.
 */
UCLASS()
class UE5_BASICS_API USoccerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Inizializza porte, player e colorazioni in base alla vicinanza
	UFUNCTION(BlueprintCallable, Category = "Soccer|Setup")
	void InitializeMatch();

	void RegisterPlayer(ATP_TestCharacter* Player);
	void UnregisterPlayer(ATP_TestCharacter* Player);
	void RegisterGoal(AHitCounter* Goal);
	void UnregisterGoal(AHitCounter* Goal);

	UFUNCTION(BlueprintPure, Category = "Soccer|Team")
	AHitCounter* GetGoalForPlayer(ATP_TestCharacter* Player) const;

	// Configura l'Actor segnaposto da cui ricavare il transform di spawn
	UFUNCTION(BlueprintCallable, Category = "Soccer|Spawn")
	void SetBallSpawnPointActor(AActor* SpawnActor);

	// Registra un goal considerando la porta colpita (per rilevare goal vs autogoal)
	UFUNCTION(BlueprintCallable, Category = "Soccer|Score")
	void RegisterGoalFromBall(ATagBall* Ball, AHitCounter* GoalHit = nullptr, int32 Points = 1);

	// Registra direttamente il goal per uno specifico player
	UFUNCTION(BlueprintCallable, Category = "Soccer|Score")
	void RegisterPlayerGoal(ATP_TestCharacter* Scorer, int32 Points = 1);

	// Restituisce il punteggio di un singolo player
	UFUNCTION(BlueprintPure, Category = "Soccer|Score")
	int32 GetScoreForPlayer(const ATP_TestCharacter* Player) const;

	// Restituisce il totale dei goal segnati nella partita
	UFUNCTION(BlueprintPure, Category = "Soccer|Score")
	int32 GetTotalGoals() const { return TotalGoalsCount; }

	// Resetta punteggi e statistiche della partita
	UFUNCTION(BlueprintCallable, Category = "Soccer|Score")
	void ResetMatchStats();

	// Pianifica e genera la nuova palla allo spawn point
	UFUNCTION(BlueprintCallable, Category = "Soccer|Spawn")
	void ScheduleBallRespawn();

	UFUNCTION(BlueprintCallable, Category = "Soccer|Spawn")
	void SpawnNewBall();

	// Evento notificato ogni volta che viene segnato un goal
	UPROPERTY(BlueprintAssignable, Category = "Soccer|Events")
	FOnGoalScoredSignature OnGoalScored;

	// Evento notificato per aggiornamenti globali (es. statistiche arena)
	UPROPERTY(BlueprintAssignable, Category = "Soccer|Events")
	FOnTotalGoalsChangedSignature OnTotalGoalsChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soccer|Spawn")
	TSubclassOf<ATagBall> BallClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soccer|Spawn")
	float RespawnDelay = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soccer|Team")
	TArray<FLinearColor> TeamColors = {
		FLinearColor(0.05f, 0.25f, 1.0f, 1.0f),
		FLinearColor(1.0f, 0.05f, 0.05f, 1.0f)
	};

	UPROPERTY(BlueprintReadWrite, Category = "Soccer|Spawn")
	TWeakObjectPtr<AActor> BallSpawnPointActor;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Soccer|Score")
	TMap<ATP_TestCharacter*, int32> PlayerScores;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Soccer|Score")
	int32 TotalGoalsCount = 0;

private:
	void TryAssignGoals();
	ATP_TestCharacter* FindOpponent(ATP_TestCharacter* Player) const;

	TArray<TWeakObjectPtr<ATP_TestCharacter>> RegisteredPlayers;
	TArray<TWeakObjectPtr<AHitCounter>> RegisteredGoals;
	TMap<TWeakObjectPtr<ATP_TestCharacter>, TWeakObjectPtr<AHitCounter>> PlayerGoals;

	FTimerHandle RespawnTimerHandle;
};
