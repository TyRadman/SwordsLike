// Copyright Epic Games, Inc. All Rights Reserved.
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "SwordslikeCharacter.h"
#include "BaseEntityAnimationsComponent.h"
#include "BaseEntityData.h"
#include "BaseParryComponent.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "PlayerCombatComponent.h"
#include "PlayerHealthComponent.h"
#include "SprintComponent.h"
#include "TargetLockerComponent.h"
#include "Common/LockableTargetComponent.h"
#include "Common/WeaponHandlerComponent.h"
#include "Components/WidgetComponent.h"
#include "Swordslike/UI/HUD/HUDManager.h"
#include "Swordslike/UI/HUD/NetworkOverheadDebugger.h"
#include "Swordslike/UI/WorldUIElements/OverheadHealthBarWidget.h"
#include "Swordslike/UnitControllers/Player/LockWidgetController.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ASwordslikeCharacter::ASwordslikeCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	// GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	// GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	
	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	/////////////////////
	/// CUSTOM COMPONENTS
	/////////////////////
	TargetLockerComponent = CreateDefaultSubobject<UTargetLockerComponent>(TEXT("Player Target Locker"));
	LockIndicatorWidget = CreateDefaultSubobject<ULockWidgetController>(TEXT("Player Lock Indicator"));
	
	Combat = CreateDefaultSubobject<UPlayerCombatComponent>(TEXT("Player Combat"));
	
	Health = CreateDefaultSubobject<UPlayerHealthComponent>("Player Health");
	
	Animations = CreateDefaultSubobject<UBaseEntityAnimationsComponent>("Player Animations");

	// UI
	DebuggerText = CreateDefaultSubobject<UWidgetComponent>(TEXT("Overhead Debugger Widget"));
	DebuggerText->SetupAttachment(RootComponent);

	OverheadHealthBar = CreateDefaultSubobject<UWidgetComponent>("Overhead Health Bar");
	OverheadHealthBar->SetupAttachment(RootComponent);

	CustomMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Custom Mesh");
	CustomMesh->SetupAttachment(GetMesh());

	Sprint = CreateDefaultSubobject<USprintComponent>("Player Sprint");

	WeaponHandler = CreateDefaultSubobject<UWeaponHandlerComponent>("Player Weapon Handler");
	
	ParryComponent = CreateDefaultSubobject<UBaseParryComponent>("Player Parry");

	// VFX
	ParrySparkVFX = CreateDefaultSubobject<UNiagaraComponent>("Sparks Effect");
}

void ASwordslikeCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	if(ParryComponent)
	{
		ParryComponent->InitEntityComponent(this);

		ParryComponent->OnParryStartedEvent.AddUObject(this, &ASwordslikeCharacter::OnParryStarted);
		ParryComponent->OnParryEndedEvent.AddUObject(this, &ASwordslikeCharacter::OnParryEnded);
	}

	if(LockIndicatorWidget)
	{
		if(GetLocalRole() == ROLE_Authority)
		{
			LockIndicatorWidget->HideIndicator();
		}
	}

	if(Sprint)
	{
		Sprint->InitEntityComponent(this);
		OnJumped.AddUObject(Sprint, &USprintComponent::OnJumped);
	}
	
	if(Animations)
	{
		Animations->InitEntityComponent(this);
	}

	if(TargetLockerComponent)
	{
		TargetLockerComponent->InitEntityComponent(this);
	}

	// CACHES
	if(ULockableTargetComponent* Lockable = GetComponentByClass<ULockableTargetComponent>())
	{
		LockableTargetComponent = Lockable;
	}

	///////////////////////
	/// Widgets
	//////////////////////
	// TODO: Extract this component from the project entirely in C++ and BP
	DebuggerText->SetWidget(nullptr);

	// disable the overhead health bar if this is the AutonomousProxy client 
	if(GetLocalRole() == ROLE_AutonomousProxy)
	{
		OverheadHealthBar->SetWidget(nullptr);
	}
	else if(UOverheadHealthBarWidget* HealthBar = Cast<UOverheadHealthBarWidget>(OverheadHealthBar->GetUserWidgetObject()))
	{
		if(Health)
		{
			Health->OnEntityHealthChanged.AddUObject(HealthBar, &UOverheadHealthBarWidget::SetHealthBarValue);
			// GEngine->AddOnScreenDebugMessage(-1, 50.f,  FColor::Red, FString::Printf(TEXT("Healthbar / %s: Binded"), *GetActorNameOrLabel()));

			FString Name = FString::Printf(TEXT("%s\n%s"), *UEnum::GetValueAsString(GetLocalRole()), *GetActorNameOrLabel());
			HealthBar->SetNameValue(FText::FromString(*Name));

			HealthBar->Hide();
			
			LockableTargetComponent->OnLockableLocked.AddUObject(HealthBar, &UOverheadHealthBarWidget::Show);
			LockableTargetComponent->OnLockableUnlocked.AddUObject(HealthBar, &UOverheadHealthBarWidget::Hide);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 50.f,  FColor::Red, FString::Printf(TEXT("Healthbar / %s: Failed to bind"), *GetActorNameOrLabel()));
		}
	}

	///////////////////////
	/// Subscriptions
	///////////////////////

	// TARGET LOCKER
	if(TargetLockerComponent)
	{
		TargetLockerComponent->AddToOnLockedTarget(this, &ASwordslikeCharacter::OnTargetLockedOn);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f,  FColor::Red, FString::Printf(TEXT("TargetLockerComponent is not NULL")));
	}

	// COMBAT
	if(Combat)
	{
		if(Animations)
		{
			Combat->OnEntityRolled.AddUObject(Animations, &UBaseEntityAnimationsComponent::PlayRollMontage);
			Combat->OnEntityRolled.AddUObject(Sprint, &USprintComponent::OnRolled);
		}
		
		Combat->OnEntityRolled.AddUObject(this, &ASwordslikeCharacter::OnRollStarted);
		Combat->OnEntityRolled.AddLambda([this]()
		{
			Health->SetIsInvincible(true);
		});
		
		Combat->OnEntityRollFinished.AddUObject(this, &ASwordslikeCharacter::OnRollFinished);
		Combat->OnEntityRollFinished.AddLambda([this]()
		{
			Health->SetIsInvincible(false);
		});

		Combat->SetRollDuration(Animations->GetRollAnimationDuration());

		OnJumped.AddUObject(Combat, &UBaseCombatComponent::DisableRoll);
		OnLanded.AddUObject(Combat, &UBaseCombatComponent::EnableRoll);
	}

	// HEALTH AND EVERYTHING THAT SUBSCRIBES TO IT
	if(Health)
	{
		Health->OnEntityDeath.AddUObject(this, &ASwordslikeCharacter::OnDeath);

		if(LockableTargetComponent)
		{
			Health->OnEntityDeath.AddUObject(LockableTargetComponent, &ULockableTargetComponent::OnDeath);
		}

		// Hit react animation
		Health->OnEntityHit.AddUObject(this, &ASwordslikeCharacter::OnCharacterHit);

		if(GetLocalRole() == ROLE_AutonomousProxy)
		{
			if(AHUD* HUD = GetWorld()->GetFirstPlayerController()->GetHUD())
			{
				if(AHUDManager* HUDManager = Cast<AHUDManager>(HUD))
				{
					HUDManager->BindHealthBar(this);
					HUDManager->BindStaminaBar(this);
					HUDManager->BindPostureBar(this);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HealthComponent is NULL"));
	}
	
	///////////////////////
	/// Assignments
	///////////////////////
	if(WeaponHandler)
	{
		Combat->SetWeaponHandler(WeaponHandler);
		WeaponHandler->Setup(this);

		WeaponHandler->OnWeaponHitStarted.AddUObject(Sprint, &USprintComponent::OnWeaponHit);
	}

	// SET INITIAL VALUES
	SetInitialValues();
}

void ASwordslikeCharacter::SetInitialValues()
{
	// CHARACTER MOVEMENT STATS
	GetCharacterMovement()->MaxWalkSpeed = PlayerStats->MovementSpeed;
	GetCharacterMovement()->JumpZVelocity = PlayerStats->JumpHeight;

	if(Health)
	{
		Health->SetMaxHealth(PlayerStats->MaxHealthPoints);
		Health->FullyChargeHealth();
	}

	if(Sprint)
	{
		Sprint->SetMaxStamina(PlayerStats->MaxStamina);
		Sprint->FullyRefillStamina();
	}

	if(ParryComponent)
	{
		ParryComponent->SetMaxPosture(PlayerStats->MaxPosture);
		ParryComponent->FullyRefillPosuture();
	}
}

void ASwordslikeCharacter::SetSprintSpeed()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		Server_SetWalkSpeed(PlayerStats->SprintSpeed);
	}
	
	GetCharacterMovement()->MaxWalkSpeed = PlayerStats->SprintSpeed;
}

void ASwordslikeCharacter::ResetSpeed()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		Server_SetWalkSpeed(PlayerStats->MovementSpeed);
	}
	
	GetCharacterMovement()->MaxWalkSpeed = PlayerStats->MovementSpeed;
}

void ASwordslikeCharacter::Server_SetWalkSpeed_Implementation(float NewSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}

#pragma region Input
// Input
void ASwordslikeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASwordslikeCharacter::Move);
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASwordslikeCharacter::Look);

		if(!TargetLockerComponent)
		{
			GEngine->AddOnScreenDebugMessage(-1, 50.f,  FColor::Red, FString::Printf(TEXT("Input setup: No locker")));
		}
		
		EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Completed, TargetLockerComponent, &UTargetLockerComponent::PerformLockAction);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ASwordslikeCharacter::OnSprintStarted);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASwordslikeCharacter::OnSprintEnded);
		EnhancedInputComponent->BindAction(AttackInputAction, ETriggerEvent::Completed, this, &ASwordslikeCharacter::Attack);
		EnhancedInputComponent->BindAction(RollInputAction, ETriggerEvent::Completed, this, &ASwordslikeCharacter::Roll);
		EnhancedInputComponent->BindAction(TestInputAction, ETriggerEvent::Completed, this, &ASwordslikeCharacter::StartAttackCycle);
		EnhancedInputComponent->BindAction(ParryInputAction, ETriggerEvent::Started, this, &ASwordslikeCharacter::Parry);
		EnhancedInputComponent->BindAction(ParryInputAction, ETriggerEvent::Completed, this, &ASwordslikeCharacter::EndParry);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ASwordslikeCharacter::Jump()
{
	if(!bCanJump || Sprint->GetCurrentStamina() == 0.f)
	{
		return;
	}

	if(OnJumped.IsBound())
	{
		OnJumped.Broadcast();
	}

	Super::Jump();
}

void ASwordslikeCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if(OnLanded.IsBound())
	{
		OnLanded.Broadcast();
	}
}

void ASwordslikeCharacter::Roll()
{
	if(Sprint->GetCurrentStamina() < USprintComponent::ROLL_STAMINA_COST)
	{
		return;
	}
	
	if(Combat)
	{
		Combat->Roll();
	}
}

void ASwordslikeCharacter::Attack()
{
	if(Sprint->GetCurrentStamina() == 0.f)
	{
		return;
	}
	
	if(Combat)
	{
		Combat->AttackAction();
	}
}

void ASwordslikeCharacter::Parry()
{
	if(ParryComponent)
	{
		ParryComponent->Parry();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("No Parry Component")));
	}
}

void ASwordslikeCharacter::EndParry()
{
	if(ParryComponent)
	{
		ParryComponent->EndParry();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("No Parry Component")));
	}
}

void ASwordslikeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(GetLocalRole() == ROLE_AutonomousProxy)
	{
		// GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Max Walk Speed: %f"), GetCharacterMovement()->MaxWalkSpeed));
	}
}

void ASwordslikeCharacter::Move(const FInputActionValue& Value)
{
	if(!bCanMove)
	{
		return;
	}
	
	// input is a Vector2D
	MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASwordslikeCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
#pragma endregion 

#pragma region Externals
void ASwordslikeCharacter::OnTargetLockedOn(bool IsLockedOn)
{
	if(IsLockedOn)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
	}
	else
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
	}
}

void ASwordslikeCharacter::OnDeath(const FDamageInfo& DamageInfo)
{
	GetCharacterMovement()->DisableMovement();
	GetMesh()->SetSimulatePhysics(true);
}

void ASwordslikeCharacter::OnCharacterHit(const FDamageInfo& DamageInfo)
{
	// get the parry state
	EParryState ParryState = ParryComponent->ValidateParry(DamageInfo);

	// if there is no parry, then take normal damage
	if(ParryState == EParryState::None)
	{
		bCanJump = false;
		bCanMove = false;
		Animations->PlayHitReactMontage();
		GetWorldTimerManager().SetTimer(HitRecoveryTimer, this, &ASwordslikeCharacter::OnCharacterHitRecovered, RecoveryDuration, false);
		
		Health->AddToCurrentHealth(DamageInfo);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f,  FColor::Red, FString::Printf(TEXT("PARRIED A %s PARRY"), *UEnum::GetValueAsString(ParryState)));
		
		FVector WeaponMiddlePoint = WeaponHandler->GetWeaponMiddleLocation();
		FRotator Rotation =  GetActorForwardVector().ToOrientationRotator();
		ParrySparkVFX->SetWorldLocationAndRotation(WeaponMiddlePoint, Rotation);
		ParrySparkVFX->Activate();

		ParryComponent->InflictParryPostureDamage(WeaponHandler->GetCurrentWeapon()->PostureDamagePerHit);
	}
}

void ASwordslikeCharacter::OnCharacterHitRecovered()
{
	bCanMove = true;
	bCanJump = true;
}

void ASwordslikeCharacter::OnRollStarted()
{
	bCanJump = false;
}

void ASwordslikeCharacter::OnRollFinished()
{
	bCanJump = true;
}

void ASwordslikeCharacter::OnParryStarted()
{
	bCanJump = false;
	bCanMove = false;
}

void ASwordslikeCharacter::OnParryEnded()
{
	bCanJump = true;
	bCanMove = true;
}

void ASwordslikeCharacter::OnSprintStarted()
{
	Sprint->OnSprintStated();

	if(TargetLockerComponent->bIsLockedOnTarget)
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
	}
}

void ASwordslikeCharacter::OnSprintEnded()
{
	Sprint->OnSprintEnded();

	if(TargetLockerComponent->bIsLockedOnTarget)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
	}
}
#pragma endregion

#pragma region Getters
FVector2D ASwordslikeCharacter::GetMovementVector() const
{
	return MovementVector;
}
#pragma endregion

void ASwordslikeCharacter::StartAttackCycle()
{
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ASwordslikeCharacter::Attack, 2.f, true);
}