#include "Component/AttackComponent.h"
#include "GameFramework/Character.h"
#include "DataAsset/BaseCharacterData.h"
#include "Interface/AttackInterface.h"

#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
UAttackComponent::UAttackComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

// Called when the game starts
void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	//owner actor -> attack interface
	AttackInterface = TScriptInterface<IAttackInterface>(GetOwner());
}

void UAttackComponent::RequestAttack()
{
	const bool bCanAttack = bIsAttacking == false || bCanCombo == true;
	if (bCanAttack) Attack();
	else bSavedAttack = true;
}

void UAttackComponent::TraceHit()
{
	if (AttackInterface == nullptr) return;
	if (BaseCharacterData == nullptr) return;

	//line trace
	
	//attack interface -> GetSocketLocation
	
	const FVector& StartLocation = 
		AttackInterface->I_GetSocketLocation(BaseCharacterData->TraceStart);
	const FVector& EndLocation = 
		AttackInterface->I_GetSocketLocation(BaseCharacterData->TraceEnd);

	//Hit results
	TArray<FHitResult> HitResults;

	auto DrawType = BaseCharacterData->bDrawDebugTrace 
		? EDrawDebugTrace::ForDuration 
		: EDrawDebugTrace::None;

	bool bDoHitSomething = UKismetSystemLibrary::SphereTraceMultiForObjects(
		this,
		StartLocation,
		EndLocation,
		BaseCharacterData->TraceRadius,
		BaseCharacterData->TraceObjectTypes,
		false,
		BaseCharacterData->ActorsToIgnore,
		DrawType,
		HitResults,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		BaseCharacterData->DrawTime
	);

	if (bDoHitSomething == false) return;

	for (const FHitResult& Result : HitResults)
	{
		if (HitActors.Contains(Result.GetActor())) continue;

		HandleHitResult(Result);

		//add => emplace
		HitActors.Emplace(Result.GetActor());
	}
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("Hit Count = %d"), HitCount));
}

void UAttackComponent::HandleHitResult(const FHitResult& Result)
{
	//Result.BoneName.ToString();
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, Result.BoneName.ToString());
	//UKismetSystemLibrary::DrawDebugSphere(this, Result.ImpactPoint, 10.0f);

	//character - register delegate
	if (HitSomethingDelegate.IsBound())
		HitSomethingDelegate.Execute(Result);
	
	//inform hit target -> use Delegate
	//Character -> apply point damage
}

UAnimMontage* UAttackComponent::GetCorrectAttackMontage()
{
	if (BaseCharacterData == nullptr) return nullptr;
	if (BaseCharacterData->AttackMontages.IsEmpty()) return nullptr;

	return BaseCharacterData->AttackMontages[AttackIndex];
}

void UAttackComponent::Attack()
{
	if (AttackInterface && BaseCharacterData && GetCorrectAttackMontage())
	{
		//play attack montage
		AttackInterface->I_PlayAttackMontage(GetCorrectAttackMontage());
		bIsAttacking = true;
		bCanCombo = false;
		AttackIndex = (AttackIndex + 1) % BaseCharacterData->AttackMontages.Num();
	}
}

void UAttackComponent::SetupAttackComponent(UBaseCharacterData* BCD)
{
	BaseCharacterData = BCD;
}

void UAttackComponent::AN_EndAttack()
{
	bIsAttacking = false;
	bCanCombo = false;
	bSavedAttack = false;
	AttackIndex = 0;
}

void UAttackComponent::AN_Combo()
{
	bCanCombo = true;
	if (bSavedAttack)
	{
		RequestAttack();
		bSavedAttack = false;
	}
}

void UAttackComponent::SetupTraceHit()
{
	HitActors.Empty();
}





