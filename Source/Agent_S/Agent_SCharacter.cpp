// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Agent_SCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"

AAgent_SCharacter::AAgent_SCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Create a camera boom attached to the root (capsule)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Rotation of the character should not affect rotation of boom
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->SocketOffset = FVector(0.f,0.f,75.f);
	CameraBoom->RelativeRotation = FRotator(0.f,180.f,0.f);

	// Create a camera and attach to boom
	SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	SideViewCameraComponent->bUsePawnControlRotation = false; // We don't want the controller rotating the camera

	 // Create Sphere collider for ledge finding
	SphereTracer = CreateEditorOnlyDefaultSubobject<USphereComponent>(TEXT("SphereTracer"));
	SphereTracer->SetupAttachment(RootComponent);
	SphereTracer->bGenerateOverlapEvents = true;
	SphereTracer->SetSphereRadius(100);
	SphereTracer->bHiddenInGame = false;
	SphereTracer->SetVisibility(true);

	SphereTracer->OnComponentBeginOverlap.AddDynamic(this, &AAgent_SCharacter::LedgeBeginOverlap);
	SphereTracer->OnComponentEndOverlap.AddDynamic(this, &AAgent_SCharacter::LedgeEndOverlap);

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Face in the direction we are moving..
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->GravityScale = 2.f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MaxFlySpeed = 600.f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void AAgent_SCharacter::BeginPlay()
{
	Super::BeginPlay();

	IsHanging = false;

	//this->channel
}


//////////////////////////////////////////////////////////////////////////
// Input

void AAgent_SCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AAgent_SCharacter::StartCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AAgent_SCharacter::StopCrouch);

	PlayerInputComponent->BindAxis("MoveRight", this, &AAgent_SCharacter::MoveRight);

	//PlayerInputComponent->BindTouch(IE_Pressed, this, &AAgent_SCharacter::TouchStarted);
	//PlayerInputComponent->BindTouch(IE_Released, this, &AAgent_SCharacter::TouchStopped);
}

void AAgent_SCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCanTraceLedge)
	{
		FVector LedgeLocation;
		bool bLedgeFound	= FindLedge(LedgeLocation);
		FVector WallLocation;
		FVector WallNormal;
		bool bWallFound = FindWall(WallLocation, WallNormal);
		UE_LOG(LogTemp, Warning, TEXT("Wall Location = %s"), *(WallLocation.ToString()))
		UE_LOG(LogTemp, Warning, TEXT("Ledge Location = %s"), *(LedgeLocation.ToString()))
	}
}
void AAgent_SCharacter::StartCrouch()
{
	if (IsHanging)
	{
		DropDown();
	}
	else
	{
		Crouch();
	}
}

void AAgent_SCharacter::StopCrouch()
{
	UnCrouch();
}

void AAgent_SCharacter::MoveRight(float Value)
{
	// add movement in that direction
	if (!IsHanging)
	{
		AddMovementInput(FVector(0.0f, -1.0f, 0.0f) * Value);
	}
}

//void AAgent_SCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	// jump on any touch
//	Jump();
//}
//
//void AAgent_SCharacter::TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	StopJumping();
//}

void AAgent_SCharacter::DropDown() 
{

}

//TODO Implement FindLedge
bool AAgent_SCharacter::FindLedge(FVector& LedgeLocation)
{
	FHitResult HitResult;
	FVector StartVector;
	FVector EndVector;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(20);
	float Distance = 25;
	StartVector = GetActorLocation() + FVector(0.f, 0.f, 75.f) + GetActorForwardVector() * 50;
	EndVector = StartVector - FVector(0.f, 0.f, Distance);
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.bTraceComplex = true;

	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		StartVector,
		EndVector,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel2,
		Sphere,
		CollisionQueryParams);

	if (bHit)
	{
		LedgeLocation = HitResult.Location;
		UE_LOG(LogTemp, Warning, TEXT("Ledge Hit = %s"), *(HitResult.GetActor()->GetName()))
		UE_LOG(LogTemp, Warning, TEXT("Ledge Blocking = %s"), (HitResult.bBlockingHit ? TEXT("True") : TEXT("False")))
		DrawDebugSphere(GetWorld(), HitResult.Location, 20.f, 8, FColor::Green);
	}

	return bHit;
}

bool	AAgent_SCharacter::FindWall(FVector& WallLocation, FVector& WallNormal)
{
	FHitResult HitResult;
	FVector StartVector;
	FVector EndVector;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(20);
	float Distance = 25;
	StartVector = GetActorLocation() + FVector(0.f, 0.f, 50.f);
	EndVector = StartVector + FVector(GetActorForwardVector().X * 50, GetActorForwardVector().Y * 50, GetActorForwardVector().Z);
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.bTraceComplex = true;

	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		StartVector,
		EndVector,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel2,
		Sphere,
		CollisionQueryParams);
	ECollisionChannel TraceChannel = ECollisionChannel::ECC_GameTraceChannel2;

	if (bHit)
	{
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 20.f, 8, FColor::Green);
		WallLocation = HitResult.ImpactPoint;
		WallNormal = HitResult.Normal;
		UE_LOG(LogTemp, Warning, TEXT("Wall Hit = %s"), *(HitResult.GetActor()->GetName()))
		UE_LOG(LogTemp, Warning, TEXT("Ledge Blocking = %s"), (HitResult.bBlockingHit ? TEXT("True") : TEXT("False")))
	}

	return bHit;
}

void AAgent_SCharacter::LedgeBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	bCanTraceLedge = true;
}

void AAgent_SCharacter::LedgeEndOverlap(
	class UPrimitiveComponent* OverlappedComp,
	class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	bCanTraceLedge = false;
}