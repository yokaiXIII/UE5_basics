// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "TestHealthComponent.h"
#include "UE5_basics/TP_Character/TP_Character.h"
#include "TP_TestCharacter.generated.h"

class ATagBall;

UCLASS()
class UE5_BASICS_API ATP_TestCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	// Sets default values for this character's properties
	ATP_TestCharacter();
	
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
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Input")
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Health")
	UTestHealthComponent* HealthComponent;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaDrainPerSecond = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float AttackStaminaCost = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float AttackHitStaminaDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina", meta = (ClampMin = "0.0"))
	float StaminaToRecoverFromExhaustion = 30.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
	bool bIsExhausted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire", meta = (ClampMin = "0.0"))
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball", meta = (ClampMin = "0.0"))
	float ThrowSpeed = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float ThrowPitchAngle = 10.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Fire")
	USceneComponent* StartFire;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void NotifyControllerChanged() override;
	virtual bool CanJumpInternal_Implementation() const override;

	FTimeline AimTimeline;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* FireAction;

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
	
	/** Crouch Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* CrouchAction;
	
	UFUNCTION(BlueprintCallable, Category="Input")
	void FireFromWeapon();	
	
	// Senza palla: trace a corto raggio (furto + danno stamina).
	void Attack();

	// Con la palla: lancio (step 3).
	void ThrowBall();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ball")
	ATagBall* HeldBall = nullptr;

	friend class ATagBall;
	
	void CrouchCharacter();
	void StopCrouchCharacter();
	void ToggleCover();
	void ExitFromCover();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void AimIn();
	void AimOut();
	
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
	
	UFUNCTION(BlueprintPure, Category = "Ball")
	bool HasBall() const;

	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool IsExhausted() const { return bIsExhausted; }

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void Stun();

	UFUNCTION(BlueprintCallable, Category = "Soccer|Team")
	void SetTeamColor(const FLinearColor& TeamColor);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soccer|Team")
	TArray<FName> TeamColorParameterNames = { TEXT("BodyColor"), TEXT("Color"), TEXT("BaseColor"), TEXT("Tint") };

	UFUNCTION(BlueprintPure, Category = "Score")
	int32 GetScore() const;

private:
	UFUNCTION()
	void HandleStaminaDepleted();

	UFUNCTION()
	void HandleProgressArmLength(float Length);
	
	UFUNCTION()
	void HandleProgressCameraOffset(FVector Offset);
	
	FTimeline CrouchTimeline;
	
	UPROPERTY(BlueprintAssignable)
	FGameStateCharacter OnCharacterCrouch;
	
	UPROPERTY(BlueprintAssignable)
	FGameStateCharacter OnCharacterUncrouch;
};
