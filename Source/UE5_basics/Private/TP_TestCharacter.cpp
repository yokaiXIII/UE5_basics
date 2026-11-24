// Fill out your copyright notice in the Description page of Project Settings.


#include "TP_TestCharacter.h"
#include "TagBall.h"
#include "SoccerSubsystem.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "TestHealthComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "UE5_basics/Enemy/Enemy.h"
class UEnhancedInputLocalPlayerSubsystem;
// Sets default values
ATP_TestCharacter::ATP_TestCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	
	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false;
	
	StartFire = CreateDefaultSubobject<USceneComponent>(TEXT("StartTrace"));
	StartFire->SetupAttachment(RootComponent);
	
	HealthComponent = CreateDefaultSubobject<UTestHealthComponent>(TEXT("Health Component"));
}

// Called when the game starts or when spawned
void ATP_TestCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(HealthComponent))
	{
		HealthComponent->OnHealthToZero.AddDynamic(this, &ATP_TestCharacter::HandleStaminaDepleted);
		if (HealthComponent->GetCurrentHealth() <= 0.0f)
		{
			HandleStaminaDepleted();
		}
	}
	if (MovementCurve && OffsetCurve) 
	{
		FOnTimelineFloat ProgressFunctionLength;
		ProgressFunctionLength.BindUFunction(this, "HandleProgressArmLength");
		AimTimeline.AddInterpFloat(MovementCurve, ProgressFunctionLength);

		FOnTimelineVector ProgressFunctionOffset;
		ProgressFunctionOffset.BindUFunction(this, "HandleProgressCameraOffset");
		AimTimeline.AddInterpVector(OffsetCurve, ProgressFunctionOffset);
	}

	if (UWorld* World = GetWorld())
	{
		if (USoccerSubsystem* SoccerSubsystem = World->GetSubsystem<USoccerSubsystem>())
		{
			SoccerSubsystem->RegisterPlayer(this);
		}
	}
}

void ATP_TestCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (USoccerSubsystem* SoccerSubsystem = World->GetSubsystem<USoccerSubsystem>())
		{
			SoccerSubsystem->UnregisterPlayer(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ATP_TestCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ATP_TestCharacter::FireFromWeapon()
{
	if (HasBall())
	{
		ThrowBall();
	}
	else
	{
		Attack();
	}
}

void ATP_TestCharacter::ThrowBall()
{
	if (!IsValid(HeldBall))
	{
		return;
	}

	const FRotator ThrowRotation(
		ThrowPitchAngle,
		GetActorRotation().Yaw,
		0.0f);
	ATagBall* Ball = HeldBall;
	Ball->Release(true);

	if (UPrimitiveComponent* BallRoot = Cast<UPrimitiveComponent>(Ball->GetRootComponent()))
	{
		BallRoot->AddImpulse(
			ThrowRotation.Vector() * FMath::Max(0.0f, ThrowSpeed), NAME_None, true);
	}
}

void ATP_TestCharacter::Attack()
{
	if (bIsExhausted)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World) || !IsValid(StartFire))
	{
		return;
	}

	const float StaminaCost = FMath::Max(0.0f, AttackStaminaCost);
	if (!IsValid(HealthComponent) || !HealthComponent->HasStamina(StaminaCost))
	{
		return;
	}

	HealthComponent->ConsumeStamina(StaminaCost);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	const FVector Start = StartFire->GetComponentLocation();
	const FVector Direction = StartFire->GetForwardVector();
	const FVector End = Start + Direction * FMath::Max(0.0f, AttackRange);

	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params);

	DrawDebugLine(World, Start, End, FColor::Red, false, 1.0f, 0, 1.0f);

	if (!bHit)
	{
		return;
	}

	AActor* HitActor = Hit.GetActor();
	if (!IsValid(HitActor))
	{
		return;
	}

	ATP_TestCharacter* HitPlayer = Cast<ATP_TestCharacter>(HitActor);
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("Hit %s"), *HitActor->GetName()));
	if (IsValid(HitPlayer) && HitPlayer != this &&
		IsValid(HitPlayer->HeldBall))
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("Stealing ball from %s"), *HitPlayer->GetName()));
		HitPlayer->HeldBall->GiveTo(this);
	}

	if (IsValid(HitPlayer) && HitPlayer != this &&
		IsValid(HitPlayer->HealthComponent))
	{
		HitPlayer->HealthComponent->ConsumeStamina(AttackHitStaminaDamage);
	}
}

bool ATP_TestCharacter::HasBall() const
{
	return IsValid(HeldBall);
}

// Called every frame
void ATP_TestCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimTimeline.TickTimeline(DeltaTime);

	if (bIsExhausted && IsValid(HealthComponent))
	{
		const float MaxStamina = FMath::Max(0.0f, HealthComponent->GetMaxHealth());
		const float RecoveryThreshold = FMath::Clamp(
			StaminaToRecoverFromExhaustion, 0.0f, MaxStamina);
		if (HealthComponent->GetCurrentHealth() > 0.0f &&
			HealthComponent->GetCurrentHealth() >= RecoveryThreshold)
		{
			bIsExhausted = false;
		}
	}

	if (HasBall() && IsValid(HealthComponent))
	{
		HealthComponent->ConsumeStamina(StaminaDrainPerSecond * DeltaTime);
	}
}

void ATP_TestCharacter::HandleStaminaDepleted()
{
	bIsExhausted = true;
	GetCharacterMovement()->StopMovementImmediately();

	if (IsValid(HeldBall))
	{
		HeldBall->Drop();
	}
}

void ATP_TestCharacter::Stun()
{
	if (bIsExhausted || !IsValid(HealthComponent))
	{
		return;
	}

	const float CurrentStamina = HealthComponent->GetCurrentHealth();
	if (CurrentStamina > 0.0f)
	{
		HealthComponent->ConsumeStamina(CurrentStamina);
	}
	else
	{
		HandleStaminaDepleted();
	}
}

void ATP_TestCharacter::SetTeamColor(const FLinearColor& TeamColor)
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (IsValid(MeshComp))
	{
		const int32 NumMaterials = MeshComp->GetNumMaterials();
		for (int32 MatIdx = 0; MatIdx < NumMaterials; ++MatIdx)
		{
			UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(MeshComp->GetMaterial(MatIdx));
			if (!IsValid(DynMaterial))
			{
				DynMaterial = MeshComp->CreateAndSetMaterialInstanceDynamic(MatIdx);
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
					UE_LOG(LogTemp, Warning, TEXT("%s: nessun parametro colore vettoriale trovato sul materiale %d."), *GetName(), MatIdx);
				}
			}
		}
	}
}

int32 ATP_TestCharacter::GetScore() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const USoccerSubsystem* Subsystem = World->GetSubsystem<USoccerSubsystem>())
		{
			return Subsystem->GetScoreForPlayer(this);
		}
	}
	return 0;
}

/// Input function to override
/// @param PlayerInputComponent 
void ATP_TestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		if (IsValid(FireAction))
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ATP_TestCharacter::FireFromWeapon);
		}

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATP_TestCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ATP_TestCharacter::Look);

		// Aim
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ATP_TestCharacter::AimIn);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ATP_TestCharacter::AimOut);
		
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATP_TestCharacter::Look);
		
		// Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ATP_TestCharacter::ToggleCover);
	}
}

void ATP_TestCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ATP_TestCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ATP_TestCharacter::DoMove(float Right, float Forward)
{
	if (!bIsExhausted && GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

bool ATP_TestCharacter::CanJumpInternal_Implementation() const
{
	return !bIsExhausted && Super::CanJumpInternal_Implementation();
}

void ATP_TestCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ATP_TestCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ATP_TestCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void ATP_TestCharacter::AimIn()
{
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;
	AimTimeline.Play();
}

void ATP_TestCharacter::AimOut()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	AimTimeline.Reverse();
}

void ATP_TestCharacter::HandleProgressArmLength(float Length)
{
	CameraBoom->TargetArmLength = Length;
}

void ATP_TestCharacter::HandleProgressCameraOffset(FVector Offset) {
	CameraBoom->SocketOffset = Offset;
}

void ATP_TestCharacter::CrouchCharacter() {
	
	if (CanCrouch()) {
		GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Green, TEXT("Crouch in"));
		Crouch();
		CrouchTimeline.Play();
		OnCharacterCrouch.Broadcast();
	}
}

void ATP_TestCharacter::StopCrouchCharacter() {
	GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Green, TEXT("Uncrouch"));
	if (GetCharacterMovement()->IsCrouching()) {
		UnCrouch();
		CrouchTimeline.Reverse();
		OnCharacterUncrouch.Broadcast();
	}
}

void ATP_TestCharacter::ToggleCover() {
	// bool bIsNear = CheckAroundMe(CheckCoverRadius);
	// if (bIsNear) { // Check if cover is near
	// 	bIsInCover = !bIsInCover;
	// 	// DEBUG message
	// 	if (bIsInCover) {
	// 		GEngine->AddOnScreenDebugMessage(-1, 2.2f, FColor::Green, TEXT("Cover now!"));
	// 	} else {
	// 		GEngine->AddOnScreenDebugMessage(-1, 2.2f, FColor::Green, TEXT("Not Cover now!"));
	// 	}
	// 	
	// 	if (bIsInCover && Cover) {			
	// 		FRotator CoverRotation;
	// 		Cover->RetrieveMovementDirectionAndFacingRotation(CoverDirectionMovement, CoverRotation);
	// 		SetActorRotation(CoverRotation);
	// 		CrouchCharacter();
	// 	} else {
	// 		ExitFromCover();
	// 	}
	// }
}

void ATP_TestCharacter::ExitFromCover() {
	// bIsInCover = false;
	// StopCrouchCharacter();
}

