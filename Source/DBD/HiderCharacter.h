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
	void SetForceRotation(bool bIsSetRotation, FRotator targetRotation = FRotator::ZeroRotator);



	// 블루프린트 디테일 패널에서 우리가 만든 'Pallet' 채널을 선택할 수 있게 해주는 변수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vaulting")
	TEnumAsByte<ECollisionChannel> PalletCollisionChannel;


	// 판자 내리기 시작할 때 호출할 함수 
	UFUNCTION(BlueprintCallable, Category = "PalletDrop")
	void StartPalletDrop();
	// 판자 내리기 끝났을 때 호출할 함수 (모두 원상복구)
	UFUNCTION(BlueprintCallable, Category = "PalletDrop")
	void StopPalletDrop();

	// 판자 넘기 시작할 때 호출할 함수 (충돌 끄기, 입력 막기, 회전 고정)
	UFUNCTION(BlueprintCallable, Category = "Vaulting")
	void StartVaulting();
	// 판자 넘기가 끝났을 때 호출할 함수 (모두 원상복구)
	UFUNCTION(BlueprintCallable, Category = "Vaulting")
	void StopVaulting();
};