// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BaseCharacterData.generated.h"

class UAnimMontage;

UCLASS()
class MYTESTCPP_API UBaseCharacterData : public UDataAsset
{
	GENERATED_BODY()

public:
	//Animation Montage
	UPROPERTY(EditDefaultsOnly, Category = "Attack");
	TArray < UAnimMontage* > AttackMontages;

	//hit react montage
	UPROPERTY(EditDefaultsOnly, Category = "Hit React");
	UAnimMontage* HitReactMontage_Front;
	UPROPERTY(EditDefaultsOnly, Category = "Hit React");
	UAnimMontage* HitReactMontage_Back;
	UPROPERTY(EditDefaultsOnly, Category = "Hit React");
	UAnimMontage* HitReactMontage_Right;
	UPROPERTY(EditDefaultsOnly, Category = "Hit React");
	UAnimMontage* HitReactMontage_Left;
	
	// trace hit
	UPROPERTY(EditDefaultsOnly, Category = "Trace Hit")
	TArray < TEnumAsByte < EObjectTypeQuery > > TraceObjectTypes;
	UPROPERTY(EditDefaultsOnly, Category = "Trace Hit")
	TArray < AActor* > ActorsToIgnore;
	UPROPERTY(EditDefaultsOnly, Category = "Trace Hit")
	float TraceRadius = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Trace Hit")
	FName TraceStart;
	UPROPERTY(EditDefaultsOnly, Category = "Trace Hit")
	FName TraceEnd;

	UPROPERTY(EditDefaultsOnly, Category = "Trace Hit")
	float DrawTime = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Trace Hit")
	bool bDrawDebugTrace = true;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float Damage = 20.0f;
};
