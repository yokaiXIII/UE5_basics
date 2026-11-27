// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "UE5_basics/Other/HealthComponent.h"
#include "UE5_basics/Enemy/Enemy.h"
#include "UE5_basics/Public/HitComponent.h"
#include "UE5_basics/Public/Ball.h"
#include "UE5_basics/Subsystem/ScoreWorldSubsystem.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
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

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HitComponent = CreateDefaultSubobject<UHitComponent>(TEXT("HitComponent"));
	
	
	HealthComponent->OnHealtToZero.AddDynamic(this, &APlayerCharacter::HandleHealthToZero);
	HealthComponent->OnHealthRecovery.AddDynamic(this, &APlayerCharacter::HandleHealthRecovery);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if(MovementCurve)
	{
		FOnTimelineFloat ProgressFunctionLength;
		ProgressFunctionLength.BindUFunction(this, "HandleProgressArmLength");
		AimTimeline.AddInterpFloat(MovementCurve, ProgressFunctionLength);
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Silver, "Bind the timeline!");
	}

	if (OffsetCurve)
	{
		FOnTimelineVector OffsetFunctionLength;
		OffsetFunctionLength.BindUFunction(this, "HandleProgressCameraOffset");
		AimTimeline.AddInterpVector(OffsetCurve, OffsetFunctionLength);
	}
	

	HitComponent->OnHit.AddDynamic(this , &APlayerCharacter::HandleHit);
	
	// Physics collision
	auto mesh = GetCapsuleComponent();
	if (IsValid(mesh))
	{
		mesh->OnComponentHit.AddDynamic(this, &APlayerCharacter::OnComponentHit);
	}

	PlayerMaterialInstance = UMaterialInstanceDynamic::Create(GetMesh()->GetMaterial(0) , 0);
}


void APlayerCharacter::HandleHealthToZero()
{
	if (IsStun) return;
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, "STUNNED!!!");
	IsStun = true;
	ToggleCommands(false);
	if (IsValid(BallRetrived) && BallRetrived != nullptr)
	{
		BallRetrived->Released(FollowCamera->GetForwardVector(), true);
	}
}

void APlayerCharacter::ToggleCommands(bool status)
{
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (!IsValid(PC)) return;
	if (status)
	{
		EnableInput(PC);
	}else
	{
		DisableInput(PC);
	}
}

void APlayerCharacter::HandleHealthRecovery()
{
	if (!IsStun) return;
	if (HealthComponent->HealthPercentage() >= 1.0f)
	{
		ReturnedFullHealth();
	}
}

void APlayerCharacter::ReturnedFullHealth()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Full HEALTH!!!");
	IsStun = false;
	ToggleCommands(true);
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimTimeline.TickTimeline(DeltaTime);
	
}

void APlayerCharacter::OnComponentHit(UPrimitiveComponent* Component, AActor* OtherActor,
										UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	auto castBallActor = Cast<ABall>(OtherActor);
	if (!IsValid(castBallActor)) return;
	castBallActor->RetrivedBy(this);
}

#pragma region "Character Movement Logic"

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		// Aim
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &APlayerCharacter::AimIn);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &APlayerCharacter::AimOut);
		
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		//Shooting
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &APlayerCharacter::Throw);
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void APlayerCharacter::DoMove(float Right, float Forward)
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

void APlayerCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void APlayerCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void APlayerCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void APlayerCharacter::AimIn()
{
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	AimTimeline.Play();
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, "Aim IN");
}

void APlayerCharacter::AimOut()
{
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	AimTimeline.Reverse();
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, "Aim OUT");
}

void APlayerCharacter::HandleProgressArmLength(float Length)
{
	CameraBoom->TargetArmLength = Length;
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("New camer boom val -> %f"), Length));
}

void APlayerCharacter::HandleProgressCameraOffset(FVector Offset) {
	CameraBoom->SocketOffset = Offset;
}

#pragma endregion 

void APlayerCharacter::HandleHit(AActor* hitter)
{
	if(!IsValid(hitter))
		return;
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, "COLPITO!");
	APlayerCharacter* player = Cast<APlayerCharacter>(hitter);
	if(!IsValid(player))
		return;



	HealthComponent->GetDamageFromPlayer(player);
}


void APlayerCharacter::PickupBall(ABall* Ball)
{
	if(!IsValid(Ball))
		return;
	
	if (!Ball->RetrivedBy(this))   // la palla ha rifiutato: niente attach
		return;
	
	BallRetrived = Ball;
	

	FAttachmentTransformRules Rules(
			EAttachmentRule::SnapToTarget,   // Location
			EAttachmentRule::SnapToTarget,   // Rotation
			EAttachmentRule::KeepRelative,   // Scale
			false                            // Weld simulated bodies
		);
	BallRetrived->AttachToComponent(GetMesh() , Rules , FName(SocketName));
}


void APlayerCharacter::Throw()
{
	if (IsValid(BallRetrived) && BallRetrived != nullptr)
	{

		BallRetrived->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		BallRetrived->Released(FollowCamera->GetForwardVector());

		BallRetrived = nullptr;
		
		
		return;
	}
}

void APlayerCharacter::NotifyControllerChanged()
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

void APlayerCharacter::SetPlayerTeam(FTeam NewTeam)
{
	TeamName = NewTeam.TeamName;
	if (!IsValid(PlayerMaterialInstance))
	{
		PlayerMaterialInstance = UMaterialInstanceDynamic::Create(GetMesh()->GetMaterial(0) , 0);
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,  NewTeam.TeamColor.ToString());

	PlayerMaterialInstance->SetVectorParameterValue("BodyColor" , NewTeam.TeamColor);

	GetMesh()->SetMaterial(0 , PlayerMaterialInstance);

}

FString APlayerCharacter::GetPlayerTeam()
{
	return TeamName;
}

