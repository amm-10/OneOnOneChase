// Fill out your copyright notice in the Description page of Project Settings.

#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h" // 타이머 사용을 위해 필요
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "SeekerCharacter.h"

// Sets default values
ASeekerCharacter::ASeekerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASeekerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASeekerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ★ 핵심: bIsVaulting이 참일 때 매 프레임마다 회전값을 강제로 꽂아버림 (서버 롤백 방어)
	if (_bIsSetRotation)
	{
		SetActorRotation(_targetRotation);
	}
}

// Called to bind functionality to input
void ASeekerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ASeekerCharacter::SetForceRotation(bool bIsSetRotation, FRotator targetRotation)
{
	_bIsSetRotation = bIsSetRotation;
	_targetRotation = targetRotation;
}

// 변수 동기화 등록 (필수)
void ASeekerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASeekerCharacter, bIsStunned);
}

// =======================================================
// 서버 전용: 기절 로직 적용 및 조작 제한
// =======================================================
void ASeekerCharacter::Server_ApplyStun_Implementation(float StunTime)
{
	if (StunTime <= 0.0f) return;

	// 상태 변경
	bIsStunned = true;

	// ★ 주의점: 언리얼에서 서버(방장)는 변수를 바꿔도 OnRep 함수가 자동 실행되지 않습니다.
	// 따라서 서버 화면에서도 이펙트가 보이게 하려면 수동으로 한 번 꼭 불러줘야 합니다!
	OnRep_IsStunned();

	// 입력 제한
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}

	// 타이머 세팅 (StunTime 초 뒤에 EndStun 실행)
	GetWorldTimerManager().SetTimer(StunTimerHandle, this, &ASeekerCharacter::EndStun, StunTime, false);
}

// =======================================================
// 기절 종료: 타이머에 의해 서버에서 호출됨
// =======================================================
void ASeekerCharacter::EndStun()
{
	// 상태 원상복구
	bIsStunned = false;

	// ★ 스턴이 끝날 때도 서버 화면 갱신을 위해 수동 호출
	OnRep_IsStunned();

	// 입력 복구
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		EnableInput(PC);
	}
}


void ASeekerCharacter::StartDestroyPallet()
{
	// 블루프린트에서 달았던 OR (Is Locally Controlled || Has Authority) 노드와 완전히 같은 역할!
	if (IsLocallyControlled() || HasAuthority())
	{
		// 1. 회전 고정 켜기 (Event Tick에서 돌아가도록)
		//SetForceRotation(true, InTargetRotation);

		// 2. 판자 충돌 무시 (Ignore)
	/*	if (UCapsuleComponent* Cap = GetCapsuleComponent())
		{
			Cap->SetCollisionResponseToChannel(PalletCollisionChannel, ECR_Ignore);
		}*/
		// 입력 제한
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			DisableInput(PC);
		}
		//// 3. 키보드/마우스 이동 입력 막기
		//if (Controller)
		//{
		//	Controller->SetIgnoreMoveInput(true);
		//}
	}
}

void ASeekerCharacter::StopDestroyPallet()
{
	if (IsLocallyControlled() || HasAuthority())
	{
		// 1. 회전 고정 끄기
		SetForceRotation(false);

		//// 2. 판자 충돌 원상복구 (Block)
		//if (UCapsuleComponent* Cap = GetCapsuleComponent())
		//{
		//	Cap->SetCollisionResponseToChannel(PalletCollisionChannel, ECR_Block);
		//}
		// 입력 복구
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			EnableInput(PC);
		}
		//// 3. 키보드/마우스 이동 입력 복구
		//if (Controller)
		//{
		//	Controller->SetIgnoreMoveInput(false);
		//}
	}
}


// =======================================================
// 동기화 알림: 클라이언트들의 화면에서 자동으로 실행됨
// =======================================================
void ASeekerCharacter::OnRep_IsStunned()
{
	BP_OnStunStateChanged(bIsStunned);
}