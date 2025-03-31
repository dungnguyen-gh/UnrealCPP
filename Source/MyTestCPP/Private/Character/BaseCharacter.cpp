// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BaseCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "DataAsset/EnhancedInputData.h"
#include "DataAsset/BaseCharacterData.h"

#include "Component/AttackComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//spring arm
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->TargetArmLength = 400.0f;
	SpringArmComponent->AddLocalOffset(FVector(0.0f, 0.0f, 40.0f));

	//follow Camera - u camera component
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;

	//u attack component - actor component
	AttackComponent = CreateDefaultSubobject<UAttackComponent>(TEXT("Attack Component"));

	//boolean
	bUseControllerRotationYaw = false; 
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate.Yaw = 540.0;
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//add mapping context
	AddMappingContextForCharacter();

	//enhancecd input component
	UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (EnhancedInputData == nullptr)
		return;
	// I A Look -> Event Function -> Bind input action
	EnhancedInputComponent->BindAction(EnhancedInputData->IA_Look, ETriggerEvent::Triggered, this, &ABaseCharacter::Look); //& is address of function
	EnhancedInputComponent->BindAction(EnhancedInputData->IA_Move, ETriggerEvent::Triggered, this, &ABaseCharacter::Move);
	EnhancedInputComponent->BindAction(EnhancedInputData->IA_Attack, ETriggerEvent::Started, this, &ABaseCharacter::AttackPressed);
}

void ABaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	//attack component
	if (AttackComponent)
	{
		AttackComponent->HitSomethingDelegate.BindDynamic(this, &ABaseCharacter::HandleHitSomething);
		AttackComponent->SetupAttackComponent(BaseCharacterData);
	}
		
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	//bind delegate receive damage
	OnTakePointDamage.AddDynamic(this, &ABaseCharacter::HandleTakePointDamage);
}

#pragma region AttackInterface

void ABaseCharacter::I_PlayAttackMontage(UAnimMontage* AttackMontage)
{
	PlayAnimMontage(AttackMontage);
}

void ABaseCharacter::I_AN_EndAttack()
{
	if (AttackComponent)
		AttackComponent->AN_EndAttack();
}

void ABaseCharacter::I_AN_Combo()
{
	if (AttackComponent)
		AttackComponent->AN_Combo();
}

FVector ABaseCharacter::I_GetSocketLocation(const FName& SocketName) const
{
	if (GetMesh() == nullptr) return FVector();
	//						socket name
	return GetMesh()->GetSocketLocation(SocketName);
}

void ABaseCharacter::I_ANS_BeginTraceHit()
{
	if (AttackComponent)
		AttackComponent->SetupTraceHit();
}

void ABaseCharacter::I_ANS_TraceHit()
{
	if (AttackComponent)
		AttackComponent->TraceHit();
}

#pragma endregion


void ABaseCharacter::AddMappingContextForCharacter()
{
	//local player
	//player controller
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	//guard
	if (PlayerController == nullptr)
		return;
	UEnhancedInputLocalPlayerSubsystem* SubSystem
		= ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>
		(PlayerController->GetLocalPlayer());

	/*if (SubSystem == nullptr)
	return;*/

	//input mapping context
	if (SubSystem && EnhancedInputData)
		SubSystem->AddMappingContext(EnhancedInputData->InputMappingContext, 0);
}



UAnimMontage* ABaseCharacter::GetCorrectHitReactMontage(const FVector& AttackDirection) const
{
	if (BaseCharacterData == nullptr) return nullptr;
	const auto Dot = FVector::DotProduct(AttackDirection, GetActorForwardVector());
	const bool bShouldUseDot = FMath::Abs(Dot) > 0.5;

	if (bShouldUseDot)
	{
		//1=back & -1=front
		if (Dot > 0.0)
			return BaseCharacterData->HitReactMontage_Back;
		else
			return BaseCharacterData->HitReactMontage_Front;
	}
	else
	{
		const FVector Cross = FVector::CrossProduct(AttackDirection, GetActorForwardVector());
		//cross.z>0 right & cross.z<0 left
		if (Cross.Z > 0.0)
			return BaseCharacterData->HitReactMontage_Right;
		else
			return BaseCharacterData->HitReactMontage_Left;
	}
	return nullptr;
}

void ABaseCharacter::Look(const FInputActionValue& Value)
{
	/*if (GEngine)
		GEngine->AddOnScreenDebugMessage(
			-1, 
			1.0f, 
			FColor::Cyan, 
			TEXT("Look called")
		);*/

	//FSTRUCT Value. -> X, Y 
	const FVector2D LookValue = Value.Get<FVector2D>();

	if (LookValue.X != 0.0)
		AddControllerYawInput(LookValue.X);

	if (LookValue.Y != 0.0)
		AddControllerPitchInput(LookValue.Y);
	//float 32bit double(float) 64bit
}

void ABaseCharacter::Move(const FInputActionValue& Value)
{
	//FSTRUCT Value. -> X, Y 
	const FVector2D ActionValue = Value.Get<FVector2D>();

	//GetControlRotation().Yaw;
	//											y-pitch		z-yaw				x-roll
	const FRotator MyControllerRotation = FRotator(0.0, GetControlRotation().Yaw , 0.0);

	//forward backward
	const FVector ForwardDirection = 
		MyControllerRotation.RotateVector(FVector::ForwardVector);

	if (ActionValue.Y != 0.0)
		AddMovementInput(ForwardDirection, ActionValue.Y);

	//right left
	const FVector RightDirection =
		MyControllerRotation.RotateVector(FVector::RightVector);

	if (ActionValue.X != 0.0)
		AddMovementInput(RightDirection, ActionValue.X);
}

void ABaseCharacter::AttackPressed()
{
	//PlayAnimMontage(AttackMontage);
	if (AttackComponent)
		AttackComponent->RequestAttack();
}

void ABaseCharacter::HandleHitSomething(const FHitResult& HitResult)
{
	if (BaseCharacterData == nullptr) return;

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.0f,
			FColor::Cyan,
			TEXT("Handle Hit Something")
		);

	//gameplay statics kitmes
	//apply point damage
	auto HitActor = HitResult.GetActor();

	if (HitActor == nullptr) return;

	const auto AttackDirection = UKismetMathLibrary::GetDirectionUnitVector(
		GetActorLocation(),
		HitActor->GetActorLocation()
		);

	UGameplayStatics::ApplyPointDamage(
		HitActor,
		BaseCharacterData->Damage,
		AttackDirection,
		HitResult,
		GetController(),
		this,
		UDamageType::StaticClass()
	);
}

void ABaseCharacter::HandleTakePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	//hit react animation montage
	if (BaseCharacterData) 
	{
		PlayAnimMontage(GetCorrectHitReactMontage(ShotFromDirection));
		CombatState = ECombatState::Beaten;
	}
		
}

