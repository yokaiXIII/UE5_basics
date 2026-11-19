// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ScoreWorldSubsystem.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, int32, NewScore);

UENUM(BlueprintType)
enum class ETeam : uint8
{
	Team1,
	Team2,
	Team3,
	Team4,
	
	MAX      UMETA(Hidden)   // optional sentinel, hidden in the editor
};

UCLASS()
class UE5_BASICS_API UScoreWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection);
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable, Category="Score System")
	void AddScore(AActor* ScorerActor, int amount);
	
	UFUNCTION(BlueprintCallable, Category="Score System")
	int GetScore() const{ return currentScore; };
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Score System")
	FOnScoreChanged OnScoreChanged;
	
private:
	UPROPERTY()
	TMap<ETeam, int32> scoreMap;
	
	UPROPERTY()
	int currentScore = 0; 
};
