// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomHealthComponent.h"
#include "InputAction.h"
#include "Camera/CameraComponent.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UE5_basics/Other/CoverActor.h"
#include "MyCustomCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UEnhancedInputComponent;
class UCustomHealthComponent;
class UHitableActor;
class UDamageComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAnimStateCharacter);
UCLASS()
class UE5_BASICS_API AMyCustomCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCustomCharacter();
	
	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* MovementCurve;

	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveVector* OffsetCurve;
	
	
	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* CrouchCurve;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	// Health component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCustomHealthComponent* HealthComponent;

	// Hit component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UHitableActor* HitableComponent;
	
	// Hit component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UDamageComponent* DamageComponent;

	// Shooting variables
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int MaxRange = 1000;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FHitResult Hit;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool IsShooting = false;
	
	/** Broadcasted when character start aiming */
	UPROPERTY(BlueprintAssignable)
	FAnimStateCharacter OnCharacterAim;
	/** Broadcsted when character stop aiming */
	UPROPERTY(BlueprintAssignable)
	FAnimStateCharacter OnCharacterStopAim;
	/** Broadcasted when character crouching */
	UPROPERTY(BlueprintAssignable)
	FAnimStateCharacter OnCharacterCrouch;
	/** Broadcasted when character stop crouching */
	UPROPERTY(BlueprintAssignable)
	FAnimStateCharacter OnCharacterUncrouch;
	
	/** Walk speed while aiming. */
	UPROPERTY(EditAnywhere, Category = "Aim")
	float MaxSpeedAiming = 150.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinCrouchDistance = 30.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FTimeline AimTimeline;
	/** Timeline used to crouch character*/
	FTimeline CrouchTimeline;

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
	
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	virtual void NotifyControllerChanged() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void AimIn();
	void AimOut();
	void FireWeapon();

	void StopShoot();
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

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

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void Shoot();

	UPROPERTY(BlueprintReadWrite)
	bool IsStunned = false;

private:
	UFUNCTION()
	void HandleProgressArmLength(float Length);
	
	UFUNCTION()
	void HandleProgressCameraOffset(FVector Offset);

	UFUNCTION()
	void BulletTrace();
	void CrouchCharacter();
	void StopCrouchCharacter();
	void ToggleCover();
	void ExitFromCover();
	bool CheckAroundMe(float Radius);

	bool bIsAiming;
	bool bIsInCover;
	
	float MaxSpeedWalkingOrig;
	
	/** Pointer to cover*/
	ACoverActor* Cover;
	
	/** This variable stores the allowed movement while player is covering*/
	FVector CoverDirectionMovement;
	
	float CheckCoverRadius = 60.0f;
};
