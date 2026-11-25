// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "HitCounter.generated.h"

class ATagBall;
class ATP_TestCharacter;
class UStaticMeshComponent;

UCLASS()
class UE5_BASICS_API AHitCounter : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHitCounter();
	
	UFUNCTION()
	void HandleActorHit(AActor* SelfActor, AActor* OtherActor,
						FVector NormalImpulse, const FHitResult& Hit);

	void RegisterBallHit(ATagBall* Ball);
	void AddScore(int32 Points);

	UFUNCTION(BlueprintPure, Category = "Soccer|Score")
	int32 GetHitCount() const { return HitCount; }

	// Imposta il player difensore e il colore associato
	UFUNCTION(BlueprintCallable, Category = "Soccer|Team")
	void SetGoalOwner(ATP_TestCharacter* InOwner, const FLinearColor& TeamColor);

	UFUNCTION(BlueprintPure, Category = "Soccer|Team")
	ATP_TestCharacter* GetGoalOwner() const { return GoalOwner; }
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* HitBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* GoalMesh;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Soccer|Team")
	ATP_TestCharacter* GoalOwner = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soccer|Team")
	TArray<FName> TeamColorParameterNames = { TEXT("BodyColor"), TEXT("Color"), TEXT("BaseColor"), TEXT("Tint") };
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Soccer|Score")
	int32 HitCount = 0;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
