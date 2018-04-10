// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Agent_SCharacter.generated.h"

UCLASS(config=Game)
class AAgent_SCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	// PROPERTIES


protected:
	// METHODS
	/** Called for side to side input */
	void MoveRight(float Value);
	void StartCrouch();
	void StopCrouch();
	void DropDown();

	///** Handle touch inputs. */
	//void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	///** Handle touch stop event. */
	//void TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location);

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface


public:
	// PROPERTIES
	UPROPERTY(BlueprintReadWrite)
	bool bIsHanging = false;
	UPROPERTY(BlueprintReadWrite)
	bool bIsClimbingLedge = false;

public:
	// METHODS
	AAgent_SCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "BaseCharacter")
	void ClimbUpEvent();
	UFUNCTION(BlueprintImplementableEvent, Category = "BaseCharacter")
	void GrabLedgeEvent();

	/** Returns SideViewCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetSideViewCameraComponent() const { return SideViewCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

private:
	// PROPERTIES
	bool bCanTraceLedge = false;

	//Default Sub-Objects
	/** Side view camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* SideViewCameraComponent;
	/** Camera boom positioning the camera beside the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	// Sphere collider used for ledge detection
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	class USphereComponent* SphereTracer;


private:
	// METHODS
	bool IsLedgeInRange(FVector LedgeHeight);
	void AgentSJump();

	UFUNCTION(BlueprintCallable)
	bool FindLedge(FVector& LedgeLocation);
	UFUNCTION(BlueprintCallable)
	bool	FindWall(FVector& WallLocation, FVector& WallNormal);
	UFUNCTION()
	void LedgeBeginOverlap(
		class UPrimitiveComponent* OverlappedComp,
		class AActor* OtherActor, 
		class UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult);
	UFUNCTION()
		void LedgeEndOverlap(
			class UPrimitiveComponent* OverlappedComp,
			class AActor* OtherActor,
			class UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex);
};
