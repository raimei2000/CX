// Copyright Epic Games, Inc. All Rights Reserved.

#include "CXPawn.h"
#include "CXWheelFront.h"
#include "CXWheelRear.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerState/CXPlayerState.h"
#include "GameState/CXGameState.h"
#include "Engine/DamageEvents.h"
#include "Item/CXMineItem.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

DEFINE_LOG_CATEGORY(LogTemplateVehicle);

ACXPawn::ACXPawn()
{
	// construct the front camera boom
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	// construct the back camera boom
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	// Configure the car mesh
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	// get the Chaos Wheeled movement component
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

}

void ACXPawn::BeginPlay()
{
	Super::BeginPlay();

	if (ACXGameState* CXGS = GetWorld()->GetGameState<ACXGameState>())
	{
        CXGS->OnGameStateUpdate.AddDynamic(this, &ACXPawn::SetCanControlVehicle);
	}
}

void ACXPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// steering 
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &ACXPawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &ACXPawn::Steering);

		// throttle 
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ACXPawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &ACXPawn::Throttle);

		// break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ACXPawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopBrake);

		// handbrake 
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopHandbrake);

		// look around 
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &ACXPawn::LookAround);

		// toggle camera 
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &ACXPawn::ToggleCamera);

		// reset the vehicle 
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &ACXPawn::ResetVehicle);

		// attack
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ACXPawn::InputAttack);

		// spawn mine
		EnhancedInputComponent->BindAction(SpawnMineAction, ETriggerEvent::Started, this, &ACXPawn::CTSSpawnMine);
	}
	else
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACXPawn::Tick(float Delta)
{
	Super::Tick(Delta);

	// add some angular damping if the vehicle is in midair
	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	// realign the camera yaw to face front
	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);

	BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));
}

float ACXPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Run only on server
	if (HasAuthority() == false) { return 0.f; }

	// Basic damage logic
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage <= 0.f)
	{
		return ActualDamage;
	}

	if (DamageCauser == this)
	{
		return 0.f;
	}

	ACXPawn* Attacker = Cast<ACXPawn>(DamageCauser);

	if (IsValid(Attacker) == false) { return 0.f; }

	ACXPlayerState* VictimPS = GetPlayerState<ACXPlayerState>();
	ACXPlayerState* AttackerPS = Attacker->GetPlayerState<ACXPlayerState>();

	const int32 TransferCoinAmount = (int32)ActualDamage;
	if (VictimPS->TryRemoveCoins(TransferCoinAmount))
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("%s transfferd %d coins to %s"), *GetNameSafe(VictimPS), TransferCoinAmount, *GetNameSafe(AttackerPS));
		AttackerPS->AddCoins(TransferCoinAmount);
	}
	else
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("%s don't have enough coins"), *GetNameSafe(VictimPS));
	}

	// When hit on wheels
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		static const TSet<FName> WheelBoneNames = {
			FName("Phys_Wheel_FL"),
			FName("Phys_Wheel_FR"),
			FName("Phys_Wheel_BL"),
			FName("Phys_Wheel_BR")
		};

		const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);

		if (WheelBoneNames.Contains(PointDamageEvent->HitInfo.BoneName))
		{
			UE_LOG(LogTemp, Warning, TEXT("Wheel Hit!!"));

			ACXPlayerState* CXPS = GetPlayerState<ACXPlayerState>();
			if (IsValid(CXPS))
			{
				CXPS->ReduceBonusMaxSpeed(5.f);
			}
		}
	}
	
	return ActualDamage;
}

void ACXPawn::STCHitByMine_Implementation()
{
	UE_LOG(LogTemplateVehicle, Warning, TEXT("Mine alert"));

	bThrottleBroken = true;
	GetWorld()->GetTimerManager().SetTimer(ThrottleRecoveryTimerHandle, this, &ACXPawn::RecoverThrottle, 3.f, false);
}

void ACXPawn::RecoverThrottle()
{
	bThrottleBroken = false;
}

void ACXPawn::MulticastHitEffect_Implementation(const FVector& TraceStart, const FVector& TraceEnd, bool bHitVehicle, const FVector& ImpactPoint, const FVector& ImpactNormal)
{
	UWorld* World = GetWorld();
	DrawDebugLine(World, TraceStart, TraceEnd, FColor::Red, false, 0.4f, 0, 5.0f);

	if (!bHitVehicle) { return; }

	UGameplayStatics::SpawnEmitterAtLocation(World, HitParticle, ImpactPoint, ImpactNormal.Rotation());
}

void ACXPawn::SetCanControlVehicle(EGameState CurrentGameState)
{
	if (CurrentGameState == EGameState::Playing)
	{
		// Deactivate the handbrake
		ChaosVehicleMovement->SetHandbrakeInput(false);
		BrakeLights(false);

        bCanControlVehicle = true;
	}
	else
	{
		// Activate the handbrake
		ChaosVehicleMovement->SetHandbrakeInput(true);
		BrakeLights(true);

        bCanControlVehicle = false;
	}
}

void ACXPawn::Steering(const FInputActionValue& Value)
{
    if (bCanControlVehicle == false) { return; }

	// get the input magnitude for steering
	float SteeringValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

void ACXPawn::Throttle(const FInputActionValue& Value)
{
	// get the input magnitude for the throttle
	float ThrottleValue = Value.Get<float>();

	if (bCanControlVehicle == false || bThrottleBroken == true) { ThrottleValue = 0.f; }

	// add the input
	ChaosVehicleMovement->SetThrottleInput(ThrottleValue);
}

void ACXPawn::Brake(const FInputActionValue& Value)
{
	if (bCanControlVehicle == false) { return; }

	// get the input magnitude for the brakes
	float BreakValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetBrakeInput(BreakValue);
}

void ACXPawn::StartBrake(const FInputActionValue& Value)
{
	if (bCanControlVehicle == false) { return; }

	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACXPawn::StopBrake(const FInputActionValue& Value)
{
	if (bCanControlVehicle == false) { return; }

	// call the Blueprint hook for the break lights
	BrakeLights(false);

	// reset brake input to zero
	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void ACXPawn::StartHandbrake(const FInputActionValue& Value)
{
	if (bCanControlVehicle == false) { return; }

	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(true);

	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACXPawn::StopHandbrake(const FInputActionValue& Value)
{
	if (bCanControlVehicle == false) { return; }

	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(false);

	// call the Blueprint hook for the break lights
	BrakeLights(false);
}

void ACXPawn::LookAround(const FInputActionValue& Value)
{
	// get the flat angle value for the input 
	float LookValue = Value.Get<float>();

	// add the input
	BackSpringArm->AddLocalRotation(FRotator(0.0f, LookValue, 0.0f));
}

void ACXPawn::ToggleCamera(const FInputActionValue& Value)
{
	// toggle the active camera flag
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void ACXPawn::ResetVehicle(const FInputActionValue& Value)
{
	if (bCanControlVehicle == false) { return; }

	// reset to a location slightly above our current one
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	// reset to our yaw. Ignore pitch and roll
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;
	
	// teleport the actor to the reset spot and reset physics
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);

	UE_LOG(LogTemplateVehicle, Error, TEXT("Reset Vehicle"));
}

void ACXPawn::CTSSpawnMine_Implementation(const FInputActionValue& Value)
{
	if (IsValid(MineClass) == false) { return; }
	
	FVector SpawnLocation = GetActorLocation();
	FVector BackwardVector = (-1) * GetActorForwardVector();
	SpawnLocation += BackwardVector * 600.f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	GetWorld()->SpawnActor<ACXMineItem>(MineClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
}

void ACXPawn::InputAttack(const FInputActionValue& Value)
{
	if (bCanControlVehicle == false) { return; }

	CTSAttack();
}

void ACXPawn::CTSAttack_Implementation()
{
	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * 1000.f;
	
	UWorld* World = GetWorld();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult HitResult;
	bool bHit = World->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, Params);

    bool bHitVehicle = false;

	if (bHit)
	{
		AActor* Actor = HitResult.GetActor();
		UE_LOG(LogTemplateVehicle, Error, TEXT("Attack trace hit: %s"), *GetNameSafe(Actor));

		if (ACXPawn* CXPawn = Cast<ACXPawn>(Actor))
		{
            bHitVehicle = true;
            UGameplayStatics::ApplyPointDamage(CXPawn, 5.f, GetActorForwardVector(), HitResult, GetController(), this, nullptr);
		}
	}

	MulticastHitEffect(Start, End, bHitVehicle, HitResult.ImpactPoint, HitResult.ImpactNormal);
}


#undef LOCTEXT_NAMESPACE