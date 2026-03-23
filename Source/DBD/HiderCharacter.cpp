// Fill out your copyright notice in the Description page of Project Settings.
#include "HiderCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Controller.h"

// Sets default values
AHiderCharacter::AHiderCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 변수 초기화
	_bIsSetRotation = false;
	_targetRotation = FRotator::ZeroRotator;

	// 기본값을 하나 넣어줍니다. (나중에 블루프린트에서 Pallet 채널로 바꿀 겁니다)
	PalletCollisionChannel = ECC_WorldDynamic;
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


void AHiderCharacter::StartPalletDrop()
{
	// 블루프린트에서 달았던 OR (Is Locally Controlled || Has Authority) 노드와 완전히 같은 역할!
	if (IsLocallyControlled() || HasAuthority())
	{
		// 키보드/마우스 이동 입력 막기
		if (Controller)
		{
			Controller->SetIgnoreMoveInput(true);
		}
	}
}

void AHiderCharacter::StopPalletDrop()
{
	if (IsLocallyControlled() || HasAuthority())
	{
		// 회전 고정 끄기
		SetForceRotation(false);

		// 키보드/마우스 이동 입력 복구
		if (Controller)
		{
			Controller->SetIgnoreMoveInput(false);
		}
	}
}

void AHiderCharacter::StartVaulting()
{
	// 블루프린트에서 달았던 OR (Is Locally Controlled || Has Authority) 노드와 완전히 같은 역할!
	if (IsLocallyControlled() || HasAuthority())
	{
		// 1. 회전 고정 켜기 (Event Tick에서 돌아가도록)
		//SetForceRotation(true, InTargetRotation);

		// 2. 판자 충돌 무시 (Ignore)
		if (UCapsuleComponent* Cap = GetCapsuleComponent())
		{
			Cap->SetCollisionResponseToChannel(PalletCollisionChannel, ECR_Ignore);
		}

		// 3. 키보드/마우스 이동 입력 막기
		if (Controller)
		{
			Controller->SetIgnoreMoveInput(true);
		}
	}
}

void AHiderCharacter::StopVaulting()
{
	if (IsLocallyControlled() || HasAuthority())
	{
		// 1. 회전 고정 끄기
		SetForceRotation(false);

		// 2. 판자 충돌 원상복구 (Block)
		if (UCapsuleComponent* Cap = GetCapsuleComponent())
		{
			Cap->SetCollisionResponseToChannel(PalletCollisionChannel, ECR_Block);
		}

		// 3. 키보드/마우스 이동 입력 복구
		if (Controller)
		{
			Controller->SetIgnoreMoveInput(false);
		}
	}
}