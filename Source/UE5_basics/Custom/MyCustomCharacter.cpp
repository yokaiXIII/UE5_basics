// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCustomCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Components/CapsuleComponent.h"
#include "Custom/FPS_Target.h"
#include "GameFramework/CharacterMovementComponent.h"


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
}

// Called every frame
void AMyCustomCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimTimeline.TickTimeline(DeltaTime);
	if (IsShooting)
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
	// signal the character to jump
	Jump();
}

void AMyCustomCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AMyCustomCharacter::Shoot()
{
	IsShooting = true;
}

void AMyCustomCharacter::AimIn()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
	AimTimeline.Play();
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, "Aim IN");
}

void AMyCustomCharacter::AimOut()
{
	GetCharacterMovement()->bOrientRotationToMovement = false;
	AimTimeline.Reverse();
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, "Aim OUT");
}

void AMyCustomCharacter::FireWeapon()
{
	BulletTrace();
	GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Red, "Firing");
	auto ActorHit = Hit.GetActor();
	if (!IsValid(ActorHit)) return;

	auto Target = Cast<AFPS_Target>(ActorHit);
	if (!IsValid(Target)) return;
	GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Yellow, "Target Found");
	Target->Hit(this);
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