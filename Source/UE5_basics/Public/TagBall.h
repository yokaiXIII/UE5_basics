#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TagBall.generated.h"

class ATP_TestCharacter;
class UPrimitiveComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class UE5_BASICS_API ATagBall : public AActor
{
	GENERATED_BODY()

public:
	ATagBall();

	// Usata sia per la raccolta sia per il furto.
	bool GiveTo(ATP_TestCharacter* NewHolder);
	void Drop();
	void Release(bool bLaunched);
	void Explode();

	UFUNCTION(BlueprintPure, Category = "Ball")
	bool IsInFlight() const { return bInFlight; }

	UFUNCTION(BlueprintPure, Category = "Ball")
	ATP_TestCharacter* GetLastThrower() const { return LastThrower; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball")
	USphereComponent* PickupSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball")
	UStaticMeshComponent* BallMesh;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ball")
	ATP_TestCharacter* Holder = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ball")
	ATP_TestCharacter* LastThrower = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ball")
	bool bInFlight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball")
	FVector CarryOffset = FVector(0.0f, 0.0f, 130.0f);

private:
	UFUNCTION()
	void HandleBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void HandleHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);
};