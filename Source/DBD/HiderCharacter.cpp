// Fill out your copyright notice in the Description page of Project Settings.
#include "HiderCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Controller.h"
#include "Net/UnrealNetwork.h" // 동기화 필수 헤더
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AHiderCharacter::AHiderCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultWalkSpeed = 600.0f;
	_bIsSetRotation = false;
	_targetRotation = FRotator::ZeroRotator;

	// 상호작용 장애물 채널
	PalletCollisionChannel = ECC_WorldDynamic;
}

// [추가] bIsInteracting 변수를 서버-클라이언트 간에 동기화시킵니다!
void AHiderCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHiderCharacter, bIsInteracting);
}

void AHiderCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AHiderCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (_bIsSetRotation)
	{
		SetActorRotation(_targetRotation);
	}
}

void AHiderCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AHiderCharacter::SetForceRotation(bool bIsSetRotation, FRotator targetRotation)
{
	_bIsSetRotation = bIsSetRotation;
	_targetRotation = targetRotation;
}


// =========================================================
// [핵심 추가] 상호작용 상태가 변하면 '모두의 컴퓨터'에서 실행되는 함수
// =========================================================
void AHiderCharacter::OnRep_IsInteracting()
{
	if (UCapsuleComponent* Cap = GetCapsuleComponent())
	{
		if (bIsInteracting)
		{
			// 상호작용 중일 땐 모두의 화면에서 충돌 무시! (러버밴딩 해결)
			Cap->SetCollisionResponseToChannel(PalletCollisionChannel, ECR_Ignore);
			// [추가] 3. 허공에 있는 동안 중력에 의해 밑으로 당겨지며 부들거리는 현상 방지!
			if (GetCharacterMovement())
			{
				GetCharacterMovement()->SetMovementMode(MOVE_Flying);
			}
		}
		else
		{
			// 1. 모두 원상복구 (Block)
			Cap->SetCollisionResponseToChannel(PalletCollisionChannel, ECR_Block);
			Cap->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

			// 2. 다시 걷기 모드로 복구 (중력 적용)
			if (GetCharacterMovement())
			{
				GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			}
		}
	}
}


// =========================================================
// 액션 함수들 (Start / Stop)
// =========================================================

void AHiderCharacter::StartPalletDrop()
{
	if (IsLocallyControlled() || HasAuthority())
	{
		bIsInteracting = true;
		OnRep_IsInteracting(); // 로컬/서버에서도 즉시 콜리전 끄기 위해 강제 호출!

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
		bIsInteracting = false;
		OnRep_IsInteracting(); // 로컬/서버 콜리전 원상복구
		SetForceRotation(false);

		if (Controller)
		{
			Controller->SetIgnoreMoveInput(false);
		}
	}
}

void AHiderCharacter::StartVaulting()
{
	if (IsLocallyControlled() || HasAuthority())
	{
		bIsInteracting = true;
		OnRep_IsInteracting(); // 여기서 콜리전 끄는 로직이 실행됨!

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
		bIsInteracting = false;
		OnRep_IsInteracting(); // 여기서 콜리전 켜는 로직이 실행됨!

		SetForceRotation(false);

		if (Controller)
		{
			Controller->SetIgnoreMoveInput(false);
		}
	}
}

void AHiderCharacter::StartClimb()
{
	if (IsLocallyControlled() || HasAuthority())
	{
		bIsInteracting = true;
		OnRep_IsInteracting();

		if (Controller)
		{
			Controller->SetIgnoreMoveInput(true);
		}
	}
}

void AHiderCharacter::StopClimb()
{
	if (IsLocallyControlled() || HasAuthority())
	{
		bIsInteracting = false;
		OnRep_IsInteracting();

		SetForceRotation(false);

		if (Controller)
		{
			Controller->SetIgnoreMoveInput(false);
		}
	}
}