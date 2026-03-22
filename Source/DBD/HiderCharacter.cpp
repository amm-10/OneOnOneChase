// Fill out your copyright notice in the Description page of Project Settings.


#include "HiderCharacter.h"

// Sets default values
AHiderCharacter::AHiderCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 변수 초기화
	_bIsSetRotation = false;
	_targetRotation = FRotator::ZeroRotator;
}

// Called when the game starts or when spawned
void AHiderCharacter::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AHiderCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ★ 핵심: bIsVaulting이 참일 때 매 프레임마다 회전값을 강제로 꽂아버림 (서버 롤백 방어)
	if (_bIsSetRotation)
	{
		SetActorRotation(_targetRotation);
	}
}

// Called to bind functionality to input
void AHiderCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// 블루프린트에서 호출할 설정 함수 구현부
void AHiderCharacter::SetForceRotation(bool bIsSetRotation, FRotator targetRotation)
{
	_bIsSetRotation = bIsSetRotation;
	_targetRotation = targetRotation;
}