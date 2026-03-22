// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HiderCharacter.generated.h"

UCLASS()
class DBD_API AHiderCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AHiderCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 판자 넘기 상태를 체크할 bool 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vaulting")
	bool _bIsSetRotation;

	// 고정시킬 목표 회전값
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vaulting")
	FRotator _targetRotation;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 블루프린트에서 회전값과 상태를 설정할 수 있는 함수
	UFUNCTION(BlueprintCallable, Category = "Vaulting")
	void SetForceRotation(bool bIsSetRotation, FRotator targetRotation);

};