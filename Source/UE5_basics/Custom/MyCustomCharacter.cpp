// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCustomCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Components/CapsuleComponent.h"
#include "Custom/DamageComponent.h"
#include "Custom/FPS_Target.h"
#include "Custom/HitableActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UE5_basics/TP_Character/TP_Character.h"


// Sets default values
AMyCustomCharacter::AMyCustomCharacter()
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

	// Register Health Component
	HealthComponent = CreateDefaultSubobject<UCustomHealthComponent>(TEXT("HealthComponent"));

	// Hit Component
	HitableComponent = CreateDefaultSubobject<UHitableActor>(TEXT("HitableComponent"));

	// Hit Component
	DamageComponent = CreateDefaultSubobject<UDamageComponent>(TEXT("DamageComponent"));
}
// Called when the game starts or when spawned
void AMyCustomCharacter::BeginPlay()
{
	Super::BeginPlay();

	if(MovementCurve && OffsetCurve)
	{
		FOnTimelineFloat ProgressFunctionLength;
		ProgressFunctionLength.BindUFunction(this, "HandleProgressArmLength");
		AimTimeline.AddInterpFloat(MovementCurve, ProgressFunctionLength);
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Silver, "Bind the timeline!");

		FOnTimelineVector ProgressFunctionVector;
		ProgressFunctionVector.BindUFunction(this, "HandleProgressCameraOffset");
		AimTimeline.AddInterpVector(OffsetCurve, ProgressFunctionVector);
	}
	
	if (CrouchCurve) {
		FOnTimelineFloat ProgressFunctionCrouch;
		ProgressFunctionCrouch.BindUFunction(this, "HandleProgressCrouch");
		CrouchTimeline.AddInterpFloat(CrouchCurve, ProgressFunctionCrouch);
	}
}

// Called every frame
void AMyCustomCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimTimeline.TickTimeline(DeltaTime);
	CrouchTimeline.TickTimeline(DeltaTime);
	if (IsShooting && !IsStunned)
		FireWeapon();
	else
		Hit.Init();
}

/// Input function to override
/// @param PlayerInputComponent 
void AMyCustomCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyCustomCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AMyCustomCharacter::Look);

		// Aim
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AMyCustomCharacter::AimIn);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AMyCustomCharacter::AimOut);
		
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyCustomCharacter::Look);

		// Shoot
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this, &AMyCustomCharacter::Shoot);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &AMyCustomCharacter::StopShoot);
		
		// Cover
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AMyCustomCharacter::ToggleCover);

		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, "Binded Actions");
	}
}

void AMyCustomCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AMyCustomCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AMyCustomCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
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

void AMyCustomCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AMyCustomCharacter::DoJumpStart()
{
	if (IsStunned) return;
	// signal the character to jump
	Jump();
}

void AMyCustomCharacter::DoJumpEnd()
{
	if (IsStunned) return;
	// signal the character to stop jumping
	StopJumping();
}

void AMyCustomCharacter::Shoot()
{
	if (IsStunned) return;
	IsShooting = true;
}

void AMyCustomCharacter::AimIn() {
	GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Green, TEXT("Aim In"));
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = MaxSpeedAiming;
	bIsAiming = true;
	if (bIsInCover) {
		GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Green, TEXT("Aim from cover"));
		StopCrouchCharacter();
	}
	AimTimeline.Play();
	OnCharacterAim.Broadcast();
}

void AMyCustomCharacter::AimOut() {
	GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Green, TEXT("Aim Out"));
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = MaxSpeedWalkingOrig;
	bIsAiming = false;
	if (bIsInCover) {
		GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Green, TEXT("Stop aim from cover"));
		CrouchCharacter();
	}
	AimTimeline.Reverse();
	OnCharacterStopAim.Broadcast();
}

void AMyCustomCharacter::FireWeapon()
{
	if (IsStunned) return;
	BulletTrace();
	GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Red, "Firing");
	auto ActorHit = Hit.GetActor();
	if (!IsValid(ActorHit)) return;

	auto TargetHitComponent = ActorHit->FindComponentByClass<UHitableActor>();
	if (!IsValid(TargetHitComponent)) return;
	GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Yellow, "Target Found");
	TargetHitComponent->Hit(this);
}

void AMyCustomCharacter::StopShoot()
{
	IsShooting = false;
}

void AMyCustomCharacter::HandleProgressArmLength(float Length)
{
	CameraBoom->TargetArmLength = Length;
}

void AMyCustomCharacter::HandleProgressCameraOffset(FVector Offset) {
	CameraBoom->SocketOffset = Offset;
}

void AMyCustomCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (!DefaultMappingContext)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, "DefaultMappingContext e' NULL!");
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, "IMC aggiunto");
		}
	}
}

void AMyCustomCharacter::BulletTrace()
{
	auto World = GetWorld();
	if (!IsValid(World)) return;
	
	auto startLocation = FollowCamera->GetComponentLocation();
	auto endLocation = startLocation + FollowCamera->GetForwardVector() * MaxRange;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(UGameplayStatics::GetPlayerPawn(World, 0));

	World->LineTraceSingleByChannel(Hit, startLocation, endLocation, ECC_Pawn, Params);
	DrawDebugLine(World, startLocation, endLocation, FColor::Red, false, 0.5, 0, 2);
}

//////////////////////////////////////////////////////////////////////////
// Mechanic: Crouch

void AMyCustomCharacter::CrouchCharacter() {
	
	if (CanCrouch()) {
		GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Green, TEXT("Crouch in"));
		Crouch();
		CrouchTimeline.Play();
		OnCharacterCrouch.Broadcast();
	}
}

void AMyCustomCharacter::StopCrouchCharacter() {
	GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Green, TEXT("Uncrouch"));
	if (GetCharacterMovement()->IsCrouching()) {
		UnCrouch();
		CrouchTimeline.Reverse();
		OnCharacterUncrouch.Broadcast();
	}
}

void AMyCustomCharacter::ToggleCover() {
	const bool bIsNear = CheckAroundMe(CheckCoverRadius);
	if (bIsNear) {
		bIsInCover = !bIsInCover;
        
		if (bIsInCover && Cover) {            
			FRotator CoverRotation;
			Cover->RetrieveMovementDirectionAndFacingRotation(this, CoverDirectionMovement, CoverRotation);
            
			// Imposta la rotazione del personaggio verso la parete
			SetActorRotation(CoverRotation);
			CrouchCharacter();
		} else {
			ExitFromCover();
		}
	}
}

void AMyCustomCharacter::ExitFromCover() {
	bIsInCover = false;
	StopCrouchCharacter();
}

bool AMyCustomCharacter::CheckAroundMe(float Radius) {
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

	FCollisionShape CollShape = FCollisionShape::MakeSphere(Radius);

	TArray<FHitResult> HitResults;

	FVector Pos = GetCapsuleComponent()->GetComponentLocation();
	bool bHit = GetWorld()->SweepMultiByChannel(HitResults, Pos, Pos, FQuat::Identity, ECC_Pawn, CollShape, Params);
	//DrawDebugSphere(GetWorld(), Pos, Radius, 12, FColor::Orange, false, 0.2f);
	
	if (bHit) {
		for (auto& HitActor : HitResults) {
			ACoverActor* HitCover = Cast<ACoverActor>(HitActor.GetActor());
			if (HitCover) {
				Cover = HitCover;
				return true;
			}
		}
	}

	return false;
}