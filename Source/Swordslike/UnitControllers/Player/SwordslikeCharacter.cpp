#include "SwordslikeCharacter.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "BaseEntityAnimationsComponent.h"
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
#include "InteractionComponent.h"
#include "LocalizationDescriptor.h"
#include "MainPlayerState.h"
#include "PlayerCombatComponent.h"
#include "PlayerHealthComponent.h"
#include "SprintComponent.h"
#include "TargetLockerComponent.h"
#include "Common/LockableTargetComponent.h"
#include "Common/WeaponHandlerComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Swordslike/GameplayGameState.h"
#include "Swordslike/SwordslikeGameInstance.h"
#include "Swordslike/AnimNotifies/ComboAnimNotify.h"
#include "Swordslike/Environment/DestructibleObject.h"
#include "Swordslike/UI/HUD/HUDManager.h"
#include "Swordslike/UI/HUD/MasterHUD.h"
#include "Swordslike/UI/HUD/HealthBars/PlayerHealthBar.h"
#include "Swordslike/UI/Menus/GameOverMenuWidget.h"
#include "Swordslike/UI/WorldUIElements/OverheadHealthBarWidget.h"
#include "Swordslike/UI/WorldUIElements/WeaponAttackIndicatorWidget.h"
#include "Swordslike/UnitControllers/Player/LockWidgetController.h"
#include "Swordslike/Utilities/UtilHelper.h"

DEFINE_LOG_CATEGORY(SwordslikeLog);

void ASwordslikeCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASwordslikeCharacter, bIsLockedOnTarget);
	DOREPLIFETIME(ASwordslikeCharacter, CurrentSpeed);
	DOREPLIFETIME(ASwordslikeCharacter, PlayerCharacterDataAsset);
}

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
	GetCharacterMovement()->AirControl = 0.35f;
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

	LockableTargetComponent = CreateDefaultSubobject<ULockableTargetComponent>(TEXT("Player Lockable"));
	LockableTargetComponent->SetupAttachment(RootComponent);

	// UI
	OverheadHealthBar = CreateDefaultSubobject<UWidgetComponent>("Overhead Health Bar");
	OverheadHealthBar->SetupAttachment(RootComponent);
	
	WeaponAttackIndicatorWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("Play Weapon Attack Indicator");
	WeaponAttackIndicatorWidgetComponent->SetupAttachment(RootComponent);

	CustomMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Custom Mesh");
	CustomMesh->SetupAttachment(GetMesh());

	Sprint = CreateDefaultSubobject<USprintComponent>("Player Sprint");

	WeaponHandler = CreateDefaultSubobject<UWeaponHandlerComponent>("Player Weapon Handler");
	
	ParryComponent = CreateDefaultSubobject<UBaseParryComponent>("Player Parry");
	
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>("Player Interaction");
	
	DestructibleCollider = CreateDefaultSubobject<UCapsuleComponent>("Destructible Collider");
	DestructibleCollider->SetupAttachment(CustomMesh);

	// VFX
	ParrySparkVFX = CreateDefaultSubobject<UNiagaraComponent>("Sparks Effect");
}

void ASwordslikeCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	OnPawnBeginPlay.RemoveAll(this);
}

void ASwordslikeCharacter::BeginPlay()
{
	Super::BeginPlay();

	UUtilHelper::HideCursor(GetWorld());
	
	DestructibleCollider->OnComponentBeginOverlap.AddDynamic(this, &ASwordslikeCharacter::OnDestructibleOverlapped);
	DisableDestructibleCollider();
	
	if(Combat)
	{
		OnJumped.AddUObject(Combat, &UBaseCombatComponent::DisableRoll);
		OnLanded.AddUObject(Combat, &UBaseCombatComponent::EnableRoll);
	}
	
	if(Sprint)
	{
		OnJumped.AddUObject(Sprint, &USprintComponent::OnJumped);
	}

	SetInitialValues();
	SetDefaultReplicationProperties();
}

void ASwordslikeCharacter::CacheComponentReferences()
{
	if(GetMesh())
	{
		AnimInstance = GetMesh()->GetAnimInstance();
	}
	
	Capsule = GetComponentByClass<UCapsuleComponent>();

	if (APlayerController* ThePlayerController = Cast<APlayerController>(GetController()))
	{
		PlayerController = ThePlayerController;
		
		if (ThePlayerController->IsLocalController())
		{
			if (AHUD* HUD = ThePlayerController->GetHUD())
			{
				if(AHUDManager* HUDManagerTemp = Cast<AHUDManager>(HUD))
				{
					HUDManager = HUDManagerTemp;
					
					if(HUDManager->GetMasterHUD())
					{
						MasterHUD = HUDManager->GetMasterHUD();
					}
				}
			}
		}
	}
}

void ASwordslikeCharacter::SetInitialValues()
{
	// CHARACTER MOVEMENT STATS
	ParryVFXMap = {
		{EParryState::Normal, NormalParryParticle},
		{EParryState::Good, GoodParryParticle},
		{EParryState::Perfect, PerfectParryParticle},
			};

	if (USwordslikeGameInstance* GI = GetGameInstance<USwordslikeGameInstance>())
	{
		if (UPlayerStartCharacterDataAsset* Data = GI->GetPlayerCharacterData())
		{
			if (IsLocallyControlled())
			{
				if(!HasAuthority())
				{
					Server_SetInitialValues(Data);
				}
				else
				{
					PerformSetInitialValues(Data);
					OnRep_PlayerCharacterDataAsset();
				}
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("ERROR no UPlayerStartCharacterDataAsset SetInitialValues Player %d"), GetUniqueID()));
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("ERROR SetInitialValues Player %d"), GetUniqueID()));
	}
}

void ASwordslikeCharacter::Server_SetInitialValues_Implementation(UPlayerStartCharacterDataAsset* Data)
{
	PerformSetInitialValues(Data);
	OnRep_PlayerCharacterDataAsset();
}

void ASwordslikeCharacter::PerformSetInitialValues(UPlayerStartCharacterDataAsset* Data)
{
	PlayerCharacterDataAsset = Data;
}

void ASwordslikeCharacter::OnRep_PlayerCharacterDataAsset()
{
	if(bIsInitialized)
	{
		return;
	}
	
	if(!PlayerCharacterDataAsset || !HasActorBegunPlay())
	{
		if(!bOnBeginPlayerRegistered)
		{
			bOnBeginPlayerRegistered = true;
			OnPawnBeginPlay.AddUObject(this, &ASwordslikeCharacter::OnBeginPlay);
			return;
		}
	}

	bIsInitialized = true;
	ApplyDataValuesToPlayer();

	InitializePlayerComponents();
}

void ASwordslikeCharacter::OnBeginPlay(APawn* Pawn)
{
	OnRep_PlayerCharacterDataAsset();
}

void ASwordslikeCharacter::ApplyDataValuesToPlayer()
{
	if(CustomMesh)
	{
		CustomMesh->SetSkeletalMesh(PlayerCharacterDataAsset->CharacterSkeletalMesh);
	}

	GetCharacterMovement()->MaxWalkSpeed = PlayerCharacterDataAsset->MovementSpeed;
	GetCharacterMovement()->JumpZVelocity = PlayerCharacterDataAsset->JumpHeight;
}

void ASwordslikeCharacter::InitializePlayerComponents()
{
	CacheComponentReferences();

	// Get the overhead widget
	if(UOverheadHealthBarWidget* CastOverHeadHUD = Cast<UOverheadHealthBarWidget>(OverheadHealthBar->GetUserWidgetObject()))
	{
		OverHeadHUD = CastOverHeadHUD;
		OverHeadHUD->InitEntityComponent(this);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("NOT SET! at %d"), GetUniqueID()));
	}
	
	if(InteractionComponent)
	{
		InteractionComponent->InitEntityComponent(this);
	}

	if(WeaponHandler)
	{
		WeaponHandler->InitEntityComponent(this);
	}

	if(ParryComponent)
	{
		ParryComponent->InitEntityComponent(this);
	}

	if(Sprint)
	{
		Sprint->InitEntityComponent(this);
	}
	
	if(Animations)
	{
		Animations->InitEntityComponent(this);
	}

	if(TargetLockerComponent)
	{
		TargetLockerComponent->InitEntityComponent(this);
	}

	// Extract Weapon attack indicator
	if(WeaponAttackIndicatorWidgetComponent)
	{
		if(WeaponAttackIndicatorWidgetComponent->GetUserWidgetObject())
		{
			if(UWeaponAttackIndicatorWidget* AttackIndicator = Cast<UWeaponAttackIndicatorWidget>(WeaponAttackIndicatorWidgetComponent->GetUserWidgetObject()))
			{
				WeaponAttackIndicator = AttackIndicator;
				WeaponAttackIndicator->InitEntityComponent(this);
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("WeaponAttackIndicator NOT SET! at %d"), GetUniqueID()));
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("No widget object on [%s]"), *UEnum::GetValueAsString(GetLocalRole())));
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, TEXT("Couldn't get the indicator"));
	}

	if(Combat)
	{
		Combat->InitEntityComponent(this);
	}

	if(Health)
	{
		Health->InitEntityComponent(this);
	}

	if(LockableTargetComponent)
	{
		LockableTargetComponent->InitEntityComponent(this);
	}

	if(MasterHUD)
	{
		MasterHUD->InitEntityComponent(this);
	}

	if(LockIndicatorWidget)
	{
		LockIndicatorWidget->InitEntityComponent(this);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("LockIndicatorWidget NOT SET! at %d"), GetUniqueID()));
	}
}

/**
 * This is called in case a new player joins the game to update the other players' instances' states if their replicated properties changed for whatever reason.
 */
void ASwordslikeCharacter::SetDefaultReplicationProperties()
{
	OnRep_bIsLockedOnTarget();
}

void ASwordslikeCharacter::SetSprintSpeed()
{
	const float NewSpeed = PlayerCharacterDataAsset->SprintSpeed;
	
	if (!HasAuthority())
	{
		Server_SetWalkSpeed(NewSpeed);
	}
	else
	{
		CurrentSpeed = NewSpeed;
		OnRep_CurrentSpeed();
	}
}

void ASwordslikeCharacter::ResetSpeed()
{
	const float NewSpeed = PlayerCharacterDataAsset->MovementSpeed;
	
	if (!HasAuthority())
	{
		Server_SetWalkSpeed(NewSpeed);
	}
	else
	{
		CurrentSpeed = NewSpeed;
		OnRep_CurrentSpeed();
	}
}

void ASwordslikeCharacter::Server_SetWalkSpeed_Implementation(const float NewSpeed)
{
	CurrentSpeed = NewSpeed;
	OnRep_CurrentSpeed();
}

void ASwordslikeCharacter::OnRep_CurrentSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;
}

#pragma region Input
void ASwordslikeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (const APlayerController* ThePlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(ThePlayerController->GetLocalPlayer()))
		{
			InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
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
		EnhancedInputComponent->BindAction(TestInputAction, ETriggerEvent::Completed, this, &ASwordslikeCharacter::PerformTestActoin);
		EnhancedInputComponent->BindAction(ParryInputAction, ETriggerEvent::Started, this, &ASwordslikeCharacter::Parry);
		EnhancedInputComponent->BindAction(ParryInputAction, ETriggerEvent::Completed, this, &ASwordslikeCharacter::EndParry);
		EnhancedInputComponent->BindAction(InteractActionInput, ETriggerEvent::Completed, this, &ASwordslikeCharacter::Interact);
	}
	else
	{
		UE_LOG(SwordslikeLog, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ASwordslikeCharacter::Jump()
{
	if(!CanJump())
	{
		return;
	}

	if(OnJumped.IsBound())
	{
		OnJumped.Broadcast();
	}

	EnableDestructibleCollider();

	Super::Jump();
}

bool ASwordslikeCharacter::CanJump() const
{
	UE_LOG(LogTemp, Warning, TEXT("State %s"), *UEnum::GetValueAsString(Combat->GetComboState()));
	return bCanJump && Sprint->GetCurrentStamina() > 0.f && Combat->GetComboState() == EComboState::Idle;
}

void ASwordslikeCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if(OnLanded.IsBound())
	{
		OnLanded.Broadcast();
	}

	DisableDestructibleCollider();
}

void ASwordslikeCharacter::Roll()
{
	if(Sprint->GetCurrentStamina() < USprintComponent::ROLL_STAMINA_COST || !bCanMove)
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
	if(Sprint->GetCurrentStamina() == 0.f || !bCanAttack)
	{
		if(bIsDebugging)
			UE_LOG(LogTemp, Warning, TEXT("Attacking blocked"));
		return;
	}
	else
	{
		if(bIsDebugging)
			UE_LOG(LogTemp, Warning, TEXT("Attacking successfully"));
	}
	
	if(Combat)
	{
		Combat->AttackAction();
	}
}

void ASwordslikeCharacter::Parry()
{
	if(!WeaponHandler->HasWeapon())
	{
		return;
	}
	
	if(ParryComponent)
	{
		SetCanJump(false);
		SetCanMove(false);
		
		ParryComponent->Parry();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("No Parry Component")));
	}
}

void ASwordslikeCharacter::EndParry()
{
	if(ParryComponent && ParryComponent->bIsParrying)
	{
		ParryComponent->EndParry();
		
		if(ParryComponent->CurrentCombatState == ECombatState::Stunned)
		{
			return;
		}
		
		SetCanJump(true);
		SetCanMove(true);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("No Parry Component")));
	}
}

void ASwordslikeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// if(DestructibleCollider->GetCollisionEnabled() == ECollisionEnabled::QueryOnly)
	// {
	// 	const FVector Location = DestructibleCollider->GetComponentLocation();
	// 	const float Radius = DestructibleCollider->Bounds.SphereRadius;
	// 	DrawDebugSphere(GetWorld(), Location, Radius, 16, FColor::Red, false, 0.1f);
	// }
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
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASwordslikeCharacter::Interact()
{
	if(!InteractionComponent)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Error: No interaction component"));
		return;
	}

	InteractionComponent->Interact();
}
#pragma endregion 

#pragma region Externals
void ASwordslikeCharacter::OnTargetLockedOn(ULockableTargetComponent* Target, const bool bIsLockedOn)
{
	if(Sprint->GetIsSprintingValue() && bIsLockedOn)
	{
		return;
	}
	
	SetLockOnValue(bIsLockedOn);
}

void ASwordslikeCharacter::SetLockOnValue(const bool NewIsLockedOn)
{
	if(!HasAuthority())
	{
		Server_OnTargetLockedOn(NewIsLockedOn);
	}
	else
	{
		bIsLockedOnTarget = NewIsLockedOn;
		OnRep_bIsLockedOnTarget();
	}
}

void ASwordslikeCharacter::Server_OnTargetLockedOn_Implementation(const bool bIsLockedOn)
{
	bIsLockedOnTarget = bIsLockedOn;
	OnRep_bIsLockedOnTarget();
	// HandleOnTargetLockedOn(Target, bIsLockedOn);
}

void ASwordslikeCharacter::OnRep_bIsLockedOnTarget()
{
	GetCharacterMovement()->bOrientRotationToMovement = !bIsLockedOnTarget;
	GetCharacterMovement()->bUseControllerDesiredRotation = bIsLockedOnTarget;
}

void ASwordslikeCharacter::HandleOnTargetLockedOn(ULockableTargetComponent* Target, const bool bIsLockedOn)
{
	const FString RoleString = GetOwner()->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT");
	const FString IsLocked = bIsLockedOn ? TEXT("YES") : TEXT("NO");
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("[%s] Locked: %s"), *RoleString, *IsLocked));
}

void ASwordslikeCharacter::OnDeath()
{
	GetCharacterMovement()->DisableMovement();
	GetMesh()->SetSimulatePhysics(true);

	if (APlayerState* PS = GetPlayerState())
	{
		if (AGameplayGameState* GS = GetWorld()->GetGameState<AGameplayGameState>())
		{
			GS->ReportDeath(PS);
		}
	}
	
	if(IsLocallyControlled())
	{
		if(HUDManager)
		{
			if(UGameOverMenuWidget* GameOverMenu = HUDManager->GetGameOverMenu())
			{
				HUDManager->GetMasterHUD()->GetStatsHUD()->HideHUD();
				
				FTimerHandle GameOverTimerHandle;
				GetWorld()->GetTimerManager().SetTimer(GameOverTimerHandle, [this]()
				{
					if(!HUDManager->GetGameOverMenu()->IsInViewport())
					{
						HUDManager->GetGameOverMenu()->AddToViewport();
					}
					
					HUDManager->GetGameOverMenu()->DisplayMenu();
				},
				3.0f,
				false);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ASwordslikeCharacter ERROR: NO HUD MANAGER"))
		}
	}
}

void ASwordslikeCharacter::Client_OnDeath_Implementation()
{
}

// Always called through the server
void ASwordslikeCharacter::OnCharacterHit(const FDamageInfo& DamageInfo)
{
	PerformOnCharacterHit(DamageInfo);
}

void ASwordslikeCharacter::Server_OnCharacterHit_Implementation(const FDamageInfo& DamageInfo)
{
	PerformOnCharacterHit(DamageInfo);
}

void ASwordslikeCharacter::PerformOnCharacterHit(const FDamageInfo& DamageInfo)
{
	// posture will take damage regardless on whether the character parried or not
	if(IsLocallyControlled())
	{
		OnCharacterHitProcess(DamageInfo);
	}
	else
	{
		Client_OnCharacterHit(DamageInfo);
	}
}

void ASwordslikeCharacter::Client_OnCharacterHit_Implementation(const FDamageInfo& DamageInfo)
{
	OnCharacterHitProcess(DamageInfo);
}

// only called on the client
void ASwordslikeCharacter::OnCharacterHitProcess(const FDamageInfo& DamageInfo)
{
	const EParryState ParryState = ParryComponent->ValidateParry(DamageInfo);
	
	ParryComponent->RecievePostureDamage(DamageInfo);

	if(ParryState != EParryState::None)
	{
		ParryComponent->PlayParryEffects(DamageInfo);

		// push back the character hit when the parry isn't successful
		if(ParryState != EParryState::Perfect)
		{
			const FVector PushBackVector = -GetActorForwardVector() * ParryPushBackForce;
			CustomLaunchCharacter(this, PushBackVector);
		}
	}
	
	// if there is no parry, then take normal damage
	if(ParryState == EParryState::None)
	{
		Health->AddToCurrentHealth(DamageInfo);
			
		if(ParryComponent->CurrentCombatState == ECombatState::Normal)
		{
			SetCanJump(false);
			SetCanMove(false);
			SetCanAttack(false);
			
			if(bIsDebugging)
			UE_LOG(LogTemp, Warning, TEXT("Interrupted the attack"));
			Combat->SetComboState(EComboState::Idle);
			WeaponHandler->ResetAttackMontages();
			// Animations->PlayHitReactMontage(DamageInfo);
			GetWorldTimerManager().SetTimer(HitRecoveryTimer, this, &ASwordslikeCharacter::OnCharacterHitRecovered, RecoveryDuration, false);
		}
	}
	// if the parry is perfect, then deal damage to the posture of the attacker
	else if(ParryState == EParryState::Perfect)
	{
		if(DamageInfo.DamageInstigatorCharacter)
		{
			if(UBaseParryComponent* AttackerParry = DamageInfo.DamageInstigatorCharacter->GetParryComponent())
			{
				FDamageInfo DeflectDamageInfo;
				DeflectDamageInfo.PostureDamage = AttackerParry->MaxPosture * 0.5f;
				DeflectDamageInfo.DamageInstigator = this;
				DeflectDamageInfo.DamageInstigatorCharacter = this;
				DeflectDamageInfo.HitType = EHitType::BigHite;

				if(HasAuthority())
				{
					AttackerParry->DamagePosture(DeflectDamageInfo);
				}
				else
				{
					Server_PerformDamagePostureOnAttacker(AttackerParry, DeflectDamageInfo);
				}
			}
		}
	}

	switch (ParryState)
	{
		case EParryState::None:
				PerformCameraShake(HitCameraShake);
				break;
		case EParryState::Normal:
				PerformCameraShake(NormalParryCameraShake);
				break;
		case EParryState::Good:
				PerformCameraShake(GoodParryCameraShake);
				break;
		case EParryState::Perfect:
				PerformCameraShake(PerfectParryCameraShake);
				break;
	}
}

void ASwordslikeCharacter::Server_PerformDamagePostureOnAttacker_Implementation(UBaseParryComponent* AttackerParry,const FDamageInfo& DamageInfo)
{
	if(AttackerParry)
	{
		AttackerParry->DamagePosture(DamageInfo);
	}
}

void ASwordslikeCharacter::CustomLaunchCharacter(ASwordslikeCharacter* PushedCharacter, const FVector LaunchVector)
{
	if(HasAuthority())
	{
		PerformCustomLauchCharacter(PushedCharacter, LaunchVector);
	}
	else
	{
		Server_CustomLaunchCharacter(PushedCharacter, LaunchVector);
	}
}

void ASwordslikeCharacter::Server_CustomLaunchCharacter_Implementation(ASwordslikeCharacter* PushedCharacter, const FVector LaunchVector)
{
		PerformCustomLauchCharacter(PushedCharacter, LaunchVector);
}

void ASwordslikeCharacter::PerformCustomLauchCharacter(ASwordslikeCharacter* Character, const FVector& LaunchVector)
{
	Character->LaunchCharacter(LaunchVector, true, false);
}

void ASwordslikeCharacter::OnCharacterHitRecovered()
{
	SetCanMove(true);
	SetCanJump(true);
	SetCanAttack(true);
}

void ASwordslikeCharacter::OnAttackParried(const EParryState ParryState)
{
	const FVector WeaponMiddlePoint = WeaponHandler->GetWeaponMiddleLocation();
	const FRotator Rotation =  GetActorForwardVector().ToOrientationRotator();

	if(UNiagaraSystem* SelectedEffect = ParryVFXMap[ParryState])
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			SelectedEffect,
			WeaponMiddlePoint,
			Rotation);
	}
}

void ASwordslikeCharacter::OnStunned()
{
	if(GetWorldTimerManager().IsTimerActive(HitRecoveryTimer))
	{
		GetWorldTimerManager().ClearTimer(HitRecoveryTimer);
	}
	
	SetCanJump(false);
	SetCanMove(false);
	SetCanAttack(false);
}

void ASwordslikeCharacter::OnStunnedRecover()
{
	SetCanJump(true);
	SetCanMove(true);
	SetCanAttack(true);
	Combat->SetComboState(EComboState::Idle);
}

void ASwordslikeCharacter::OnRollStarted()
{
	// enable the collider
	// EnableDestructibleCollider();
	SetCanJump(false);
}

void ASwordslikeCharacter::OnRollFinished()
{
	// disable the collider
	// DisableDestructibleCollider();
	SetCanJump(true);
}

void ASwordslikeCharacter::OnSprintStarted()
{
	Sprint->OnSprintStated();

	SetLockOnValue(false);
}

void ASwordslikeCharacter::OnSprintEnded()
{
	Sprint->OnSprintEnded();

	RestoreCharacterRotation();
}

void ASwordslikeCharacter::RotateCharacterToDirection(const FRotator& NewRotation)
{
	if(!GetController())
	{
		return;
	}

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetController()->SetControlRotation(NewRotation);
}

void ASwordslikeCharacter::PrintOverhead(const FString& Message)
{
	OverHeadHUD->SetOverheadNameValue(FText::FromString(Message));
}

void ASwordslikeCharacter::PerformCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass)
{
	if(IsLocallyControlled())
	{
		CameraShakeProcess(ShakeClass);
	}
	else
	{
		Client_PlayCameraShake(ShakeClass);
	}
}

void ASwordslikeCharacter::Client_PlayCameraShake_Implementation(TSubclassOf<UCameraShakeBase> ShakeClass)
{
	CameraShakeProcess(ShakeClass);
}

void ASwordslikeCharacter::CameraShakeProcess(TSubclassOf<UCameraShakeBase> ShakeClass)
{
	if (PlayerController)
	{
		PlayerController->PlayerCameraManager->StartCameraShake(ShakeClass);
	}
}
#pragma endregion

void ASwordslikeCharacter::PerformTestActoin()
{
	bIsDebugging = true;
	
	if(GetWorldTimerManager().IsTimerActive(TestTimeHandle))
	{
		GetWorldTimerManager().ClearTimer(TestTimeHandle);
		return;
	}
	
	GetWorldTimerManager().SetTimer(TestTimeHandle, this, &ASwordslikeCharacter::Attack, 1.f, true);
}

void ASwordslikeCharacter::RestoreCharacterRotation()
{
	SetLockOnValue(TargetLockerComponent->GetIsLocked());
}

FString ASwordslikeCharacter::GetInteractionInput()
{
	return GetInputKey(InteractActionInput);
}

FString ASwordslikeCharacter::GetInputKey(const UInputAction* InputAction)
{
	if (!InputAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("InputAction is null."));
		return FString::Printf(TEXT("InputAction is null."));
	}

	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController not found."));
		return FString::Printf(TEXT("PlayerController not found."));
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = 
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

	if (!InputSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputLocalPlayerSubsystem not found."));
		return FString::Printf(TEXT("EnhancedInputLocalPlayerSubsystem not found."));
	}

	TArray<FKey> MappedKeys = InputSubsystem->QueryKeysMappedToAction(InputAction);

	for (const FKey& Key : MappedKeys)
	{
		UE_LOG(LogTemp, Log, TEXT("Mapped Key: %s"), *Key.ToString());
	}

	if(MappedKeys.Num() > 0)
	{
		return MappedKeys[0].ToString();
	}
	else
	{
		return FString::Printf(TEXT("NoInputKey"));
	}
}

void ASwordslikeCharacter::OnDestructibleOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor)
	{
		if(ADestructibleObject* Destructible = Cast<ADestructibleObject>(OtherActor))
		{
			FDamageInfo DamageInfo;
			DamageInfo.ImpactLocation = GetActorLocation();// + OtherActor->GetActorLocation()) / 2.0f;
			DamageInfo.DamageInstigator = this;
			DamageInfo.DamageInstigatorCharacter = this;

			if(HasAuthority())
			{
				Destructible->TakeDamage(DamageInfo);
			}
			else
			{
				Server_InflictDamageToDestructible(Destructible, DamageInfo);
			}
		}
	}
}

void ASwordslikeCharacter::EnableDestructibleCollider()
{
	DestructibleCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DestructibleCollider->SetCollisionResponseToAllChannels(ECR_Overlap);
	DestructibleCollider->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
}

void ASwordslikeCharacter::DisableDestructibleCollider()
{
	DestructibleCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASwordslikeCharacter::Server_InflictDamageToDestructible_Implementation(ADestructibleObject* Destructible, const FDamageInfo DamageInfo)
{
	Destructible->TakeDamage(DamageInfo);
}
