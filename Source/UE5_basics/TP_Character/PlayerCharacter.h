// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Camera/CameraComponent.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "UE5_basics/Public/HitComponent.h"
#include "PlayerCharacter.generated.h"

struct FTeam;
class UInputMappingContext;
class UInputAction;
class UEnhancedInputComponent;
class UHealthComponent;
class ABall;

UCLASS()
class UE5_BASICS_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	
	// Sets default values for this character's properties
	APlayerCharacter();

	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* MovementCurve;

	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveVector* OffsetCurve;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	int StaminaDamage;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString SocketName = "hand_rSocket";

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UHealthComponent* HealthComponent;
	

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UHitComponent* HitComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ABall* BallRetrived;
	
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	virtual void NotifyControllerChanged() override;
	
	FTimeline AimTimeline;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;
	
	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ShootAction;
	
	
	// Stun
	bool IsStun = false;
	
	UFUNCTION()
	void HandleHealthToZero();
	
	void ToggleCommands(bool status);

	UFUNCTION()
	void HandleHealthRecovery();
	
	void ReturnedFullHealth();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnComponentHit(UPrimitiveComponent* Component, AActor* OtherActor, UPrimitiveComponent* OtherComp,
						FVector NormalImpulse, const FHitResult& Hit);

	void AimIn();
	void AimOut();

	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void HandleHit(AActor* hitter);

	UFUNCTION(BlueprintCallable)
	void PickupBall(ABall* Ball);
	
	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();
	
	UFUNCTION(BlueprintCallable, Category="Teams")
	void SetPlayerTeam(FTeam TeamName);
	FString GetPlayerTeam();

private:
	UFUNCTION()
	void HandleProgressArmLength(float Length);
	
	UFUNCTION()
	void HandleProgressCameraOffset(FVector Offset);

	UFUNCTION()
	void Throw();
	
	UPROPERTY(VisibleAnywhere)
	FString TeamName;
};
