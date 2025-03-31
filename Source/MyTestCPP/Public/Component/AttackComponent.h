// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackComponent.generated.h"

//hit result - delegate type
DECLARE_DYNAMIC_DELEGATE_OneParam(FHitSomethingDelegate, const FHitResult&, HitResult);

class ACharacter;
class UAnimMontage;
class UBaseCharacterData;
class IAttackInterface;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYTESTCPP_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAttackComponent();

	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void RequestAttack();

	void TraceHit();


	void SetupAttackComponent(UBaseCharacterData* BCD);
	void AN_EndAttack();
	void AN_Combo();
	void SetupTraceHit();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	void Attack();
	void HandleHitResult(const FHitResult& Result);
	UAnimMontage* GetCorrectAttackMontage();

public:
	FHitSomethingDelegate HitSomethingDelegate;

private:
	//store actor implementing interface
	//template
	UPROPERTY()
	TScriptInterface<IAttackInterface> AttackInterface;

	UPROPERTY()
	UBaseCharacterData* BaseCharacterData;

	bool bIsAttacking = false;
	bool bCanCombo = false;
	bool bSavedAttack = false;
	int AttackIndex = 0;

	//trace hit
	TArray<AActor*> HitActors;

};
