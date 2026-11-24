#include "Other/HitCounter.h"

#include "TagBall.h"
#include "SoccerSubsystem.h"
#include "TP_TestCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

AHitCounter::AHitCounter()
{
	PrimaryActorTick.bCanEverTick = false;

	HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
	SetRootComponent(HitBox);

	HitBox->SetBoxExtent(FVector(100.0f));
	HitBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HitBox->SetCollisionResponseToAllChannels(ECR_Block);
	HitBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	HitBox->SetNotifyRigidBodyCollision(true);

	GoalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GoalMesh"));
	GoalMesh->SetupAttachment(HitBox);
}

void AHitCounter::BeginPlay()
{
	Super::BeginPlay();

	OnActorHit.AddDynamic(this, &AHitCounter::HandleActorHit);
	if (UWorld* World = GetWorld())
	{
		if (USoccerSubsystem* SoccerSubsystem = World->GetSubsystem<USoccerSubsystem>())
		{
			SoccerSubsystem->RegisterGoal(this);
		}
	}
}

void AHitCounter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (USoccerSubsystem* SoccerSubsystem = World->GetSubsystem<USoccerSubsystem>())
		{
			SoccerSubsystem->UnregisterGoal(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AHitCounter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHitCounter::HandleActorHit(
	AActor* SelfActor,
	AActor* OtherActor,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	RegisterBallHit(Cast<ATagBall>(OtherActor));
}

void AHitCounter::SetGoalOwner(ATP_TestCharacter* InOwner, const FLinearColor& TeamColor)
{
	GoalOwner = InOwner;

	if (IsValid(GoalMesh))
	{
		const int32 NumMaterials = GoalMesh->GetNumMaterials();
		for (int32 MatIdx = 0; MatIdx < NumMaterials; ++MatIdx)
		{
			UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(GoalMesh->GetMaterial(MatIdx));
			if (!IsValid(DynMaterial))
			{
				DynMaterial = GoalMesh->CreateAndSetMaterialInstanceDynamic(MatIdx);
			}

			if (IsValid(DynMaterial))
			{
				TArray<FMaterialParameterInfo> VectorParameterInfos;
				TArray<FGuid> VectorParameterIds;
				DynMaterial->GetAllVectorParameterInfo(VectorParameterInfos, VectorParameterIds);

				bool bFoundColorParameter = false;
				for (const FName& ParameterName : TeamColorParameterNames)
				{
					const bool bParameterExists = VectorParameterInfos.ContainsByPredicate([&ParameterName](const FMaterialParameterInfo& ParameterInfo)
					{
						return ParameterInfo.Name == ParameterName;
					});
					if (bParameterExists)
					{
						DynMaterial->SetVectorParameterValue(ParameterName, TeamColor);
						bFoundColorParameter = true;
					}
				}

				if (!bFoundColorParameter)
				{
					UE_LOG(LogTemp, Warning, TEXT("%s: nessun parametro colore vettoriale trovato sul materiale della porta %d."), *GetName(), MatIdx);
				}
			}
		}
	}
}

void AHitCounter::RegisterBallHit(ATagBall* Ball)
{
	if (!IsValid(Ball) || !Ball->IsInFlight())
	{
		return;
	}

	++HitCount;

	if (UWorld* World = GetWorld())
	{
		if (USoccerSubsystem* SoccerSubsystem = World->GetSubsystem<USoccerSubsystem>())
		{
			SoccerSubsystem->RegisterGoalFromBall(Ball, this, 1);
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Red,
			FString::Printf(TEXT("HitCount: %d"), HitCount));
	}

	UE_LOG(LogTemp, Warning, TEXT("HitCount: %d"), HitCount);
}