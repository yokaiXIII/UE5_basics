// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UE5_basics/TP_Character/PlayerCharacter.h"
#include "Ball.generated.h"

UCLASS()
class UE5_BASICS_API ABall : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABall();

	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;

	// Chiamata quando un player raccoglie la palla
	UFUNCTION(BlueprintCallable, Category = "Ball")
	bool RetrivedBy(APlayerCharacter* perpetrator);

	// Chiamata quando il player lancia/rilascia la palla
	UFUNCTION(BlueprintCallable, Category = "Ball")
	void Released(FVector forward, bool resetHolder = false);

	UFUNCTION(BlueprintPure, Category = "Ball")
	APlayerCharacter* GetHolder() const { return Holder; }
	
	float DamageFrequency = 1.0f;
	float DamageAmount = 10.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Root: corpo fisico, raggio uguale alla mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball")
	TObjectPtr<USphereComponent> Collider;

	// Area di raccolta: più grande, solo overlap
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball")
	TObjectPtr<USphereComponent> PickupTrigger;

	// Mesh aggiunta nel Blueprint (figlia del Collider), trovata in PostInitializeComponents
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ball")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ball")
	TObjectPtr<APlayerCharacter> Holder;

	// Impulso applicato al lancio
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball")
	float ThrowStrength = 1000.f;

	// Per quanto tempo dopo il lancio la palla ignora i pawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball")
	float IgnoreHolderTime = 1.0f;

private:
	FTimerHandle ColliderHolderResetHandle;
	bool bClearHolderAfterTimer = false;
	bool bIsBeingReleased = false;

	void SetBallHolder(APlayerCharacter* NewHolder);
	void ToggleCollision(bool ctx);

	void SetColliderHolderTimer();
	void StopColliderHolderTimer();
	void ColliderHolderTimerCallback();
	bool CanBePickedUpBy(const APlayerCharacter* Player) const;

	UFUNCTION()
	void OnPickupOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                     bool bFromSweep, const FHitResult& SweepResult);
	
	void SetHolderIgnoresBall(bool bIgnore);

	TEnumAsByte<ECollisionResponse> SavedCapsuleResponse = ECR_Block;
	TEnumAsByte<ECollisionResponse> SavedMeshResponse = ECR_Block;
};