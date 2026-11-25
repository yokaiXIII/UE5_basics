// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ScoreWorldSubsystem.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, int32, NewScore);

/*
 UENUM(BlueprintType)
enum class ETeam : uint8
{
	Team1,
	Team2,
	Team3,
	Team4,
	
	MAX      UMETA(Hidden)   // optional sentinel, hidden in the editor
};
*/
USTRUCT(BlueprintType)
struct FTeam
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TeamName;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Score;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TeamColor;
};
UCLASS()
class UE5_BASICS_API UScoreWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection);
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable, Category="Score System")
	void AddScore(APlayerCharacter* ScorerActor, int amount);
	
	UFUNCTION(BlueprintCallable, Category="Score System")
	int GetScore() const{ return currentScore; };
	
	UFUNCTION(BlueprintCallable, Category="Score System")
	void RegisterNewPlayer(APlayerCharacter* Player);
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Score System")
	FOnScoreChanged OnScoreChanged;
	
	UPROPERTY(BlueprintReadOnly, Category="Score System")
	TArray<FTeam> Teams;
	
	UFUNCTION(BlueprintCallable)
	TArray<FTeam> GetAllTeams();
	//UPROPERTY(VisibleAnywhere, BlueprintCallable)
    //	TMap<ETeam, int32> scoreMap;
    	
private:
	
	UPROPERTY()
	int currentScore = 0; 
	
	int TeamsCounter = 0;
};
