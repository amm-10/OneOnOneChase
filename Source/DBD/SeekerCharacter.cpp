#include "SeekerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"

// 로그 매크로
#define SCREEN_LOG(Text, Color) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, Color, Text);

ASeekerCharacter::ASeekerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 기본 이동 속도, 가속도 및 무기 설정
	DefaultWalkSpeed = 600.0f;
	DefaultAcceleration = 2048.0f;
	CurrentWeapon = EWeaponType::Melee;

	// 총(Ranged) 차징 초기값
	bIsCharging = false;
	ChargeTime = 2.0f;
	RifleRadius = 25.0f; // 반지름 10cm의 빔

	// 근접(Melee) 런지 공격 초기값
	MaxMeleeChargeTime = 1.0f;
	LungeWalkSpeed = 900.0f;
	MeleeAttackStunTime = 2.0f;
	bIsMeleeCharging = false;
}

void ASeekerCharacter::BeginPlay()
{
	Super::BeginPlay();

	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	GetComponents<USkeletalMeshComponent>(SkeletalMeshes);

	// 이름이 "Revolver"인 컴포넌트를 찾아 변수에 저장합니다.
	for (USkeletalMeshComponent* Comp : SkeletalMeshes)
	{
		if (Comp->GetName() == TEXT("Revolver"))
		{
			RevolverMesh = Comp;
			break; // 찾았으면 검색 종료
		}
	}

	if (GetMesh())
	{
		DynamicBodyMaterial = GetMesh()->CreateDynamicMaterialInstance(0);
	}
}

// =======================================================
// 애니메이션 몽타주 제어
// =======================================================

void ASeekerCharacter::PauseMeleeMontage()
{
	if (!bIsMeleeCharging) return;

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
		if (CurrentMontage)
		{
			AnimInstance->Montage_SetPlayRate(CurrentMontage, 0.0f);
		}
	}
}

void ASeekerCharacter::ResumeMeleeMontage()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
		if (CurrentMontage)
		{
			AnimInstance->Montage_SetPlayRate(CurrentMontage, 1.0f);
		}
	}
}

// =======================================================
// Tick & Input
// =======================================================

void ASeekerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (_bIsSetRotation)
	{
		SetActorRotation(_targetRotation);
	}

	if (bIsMeleeCharging)
	{
		FRotator ControlRot = GetControlRotation();
		FRotator YawRot(0.0f, ControlRot.Yaw, 0.0f);
		FVector ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);

		AddMovementInput(ForwardDir, 1.0f);
	}

	if (bIsDashing)
	{
		AddMovementInput(DashDirection, 1.0f);
	}
}

void ASeekerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASeekerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASeekerCharacter, bIsStunned);
	DOREPLIFETIME(ASeekerCharacter, CurrentWeapon);
	DOREPLIFETIME(ASeekerCharacter, bIsMeleeCharging);
	DOREPLIFETIME(ASeekerCharacter, bIsDashing);
}

// =======================================================
// 동기화 이벤트
// =======================================================

void ASeekerCharacter::OnRep_IsStunned()
{
	BP_OnStunStateChanged(bIsStunned);
}

void ASeekerCharacter::OnRep_CurrentWeapon()
{
	BP_ChangeWeaponVisibility();
}

void ASeekerCharacter::Server_SyncMeleeCharge_Implementation(bool bCharging)
{
	bIsMeleeCharging = bCharging;
	GetCharacterMovement()->MaxWalkSpeed = bCharging ? LungeWalkSpeed : DefaultWalkSpeed;
	GetCharacterMovement()->MaxAcceleration = bCharging ? 10000.0f : DefaultAcceleration;
}

// =======================================================
// 무기 스왑 로직
// =======================================================

void ASeekerCharacter::EquipMelee_Implementation()
{
	if (bIsDashing) return;

	if (bIsCharging) StopFire();

	CurrentWeapon = EWeaponType::Melee;
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	BP_ChangeWeaponVisibility();

	SCREEN_LOG(TEXT("Weapon Swap: Melee"), FColor::Cyan);
}

void ASeekerCharacter::EquipRanged_Implementation()
{
	if (bIsDashing) return;

	if (bIsMeleeCharging)
	{
		bIsMeleeCharging = false;
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
		GetCharacterMovement()->MaxAcceleration = DefaultAcceleration;
		GetWorldTimerManager().ClearTimer(MeleeChargeTimerHandle);
	}

	CurrentWeapon = EWeaponType::Ranged;
	GetCharacterMovement()->MaxWalkSpeed = 0.0f;
	BP_ChangeWeaponVisibility();

	SCREEN_LOG(TEXT("Weapon Swap: Ranged"), FColor::Orange);
}

// =======================================================
// 공격 로직
// =======================================================

void ASeekerCharacter::StartFire()
{
	if (bIsDashing) return;

	if (CurrentWeapon == EWeaponType::Melee)
	{
		if (!bIsMeleeCharging)
		{
			bIsMeleeCharging = true;

			// 로컬에서 즉시 속도 및 가속도 극대화 적용
			GetCharacterMovement()->MaxWalkSpeed = LungeWalkSpeed;
			GetCharacterMovement()->MaxAcceleration = 100000.0f;

			// 서버에 상태 동기화
			Server_SyncMeleeCharge(true);

			// 블루프린트에서 몽타주 시작 연출 (멀티캐스트 연결용)
			BP_OnMeleeAttack();

			SCREEN_LOG(TEXT("Melee Lunge Started..."), FColor::Yellow);
			GetWorldTimerManager().SetTimer(MeleeChargeTimerHandle, this, &ASeekerCharacter::ExecuteMeleeAttack, MaxMeleeChargeTime, false);
		}
	}
	else if (CurrentWeapon == EWeaponType::Ranged)
	{
		if (!bIsCharging)
		{
			bIsCharging = true;
			SCREEN_LOG(TEXT("Charging Started..."), FColor::Red);
			BP_OnChargeStart();

			GetWorldTimerManager().SetTimer(ChargeTimerHandle, this, &ASeekerCharacter::ExecuteFire, ChargeTime, false);
		}
	}
}

void ASeekerCharacter::StopFire()
{
	if (CurrentWeapon == EWeaponType::Melee && bIsMeleeCharging)
	{
		GetWorldTimerManager().ClearTimer(MeleeChargeTimerHandle);
		ExecuteMeleeAttack();
	}
	else if (CurrentWeapon == EWeaponType::Ranged && bIsCharging)
	{
		bIsCharging = false;
		GetWorldTimerManager().ClearTimer(ChargeTimerHandle);
		SCREEN_LOG(TEXT("Charge Canceled"), FColor::White);
		BP_OnChargeCancel();
	}
}

// =======================================================
// 실제 공격 발생 (타이머 완료 또는 강제 실행)
// =======================================================

void ASeekerCharacter::ExecuteMeleeAttack()
{
	if (!bIsMeleeCharging) return;

	bIsMeleeCharging = false;

	// 이동 속도 및 가속도 원상복구
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	GetCharacterMovement()->MaxAcceleration = DefaultAcceleration;

	// 서버에 복구 상태 동기화
	Server_SyncMeleeCharge(false);

	// 멈춰있던 애니메이션을 다시 재생
	ResumeMeleeMontage();

	// 히트 확인 
	CheckMeleeHit();

	SCREEN_LOG(TEXT("Melee Attack Executed!"), FColor::Red);

	// 공격 후 경직
	Server_ApplyStun(MeleeAttackStunTime);
	BP_OnMeleeAttack();
}

void ASeekerCharacter::ExecuteFire()
{
	bIsCharging = false;

	SCREEN_LOG(TEXT("Fire! Charge Rifle Shot Successful!"), FColor::Green);
	BP_OnFire();

	FireChargeRifle();
}

void ASeekerCharacter::CheckMeleeHit()
{
	// [핵심 1] 타격 판정(데미지)은 무조건 '서버'에서만 처리해야 합니다! (핵 방지)
	if (!HasAuthority()) return;

	// 1. 공격 범위 설정 (시작점, 끝점, 구체의 크기)
	FVector Start = GetActorLocation(); // 내 캐릭터 위치
	FVector ForwardVector = GetActorForwardVector();

	// 공격 거리: 내 위치에서 앞쪽으로 150.0f 만큼 나아간 곳까지 판정
	float AttackRange = 220.0f;
	FVector End = Start + (ForwardVector * AttackRange);

	// 공격 범위(두께): 반지름 60.0f 짜리 둥근 구체를 앞으로 쏩니다.
	float AttackRadius = 70.0f;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(AttackRadius);

	// 2. 트레이스 설정 (나 자신은 때리지 않게 무시 설정)
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.bTraceComplex = false; // 단순 충돌체만 검사해서 성능 확보

	// 3. 실제로 앞을 향해 구체를 발사(Sweep)합니다!
	FHitResult HitResult;
	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn, // 폰(캐릭터)들만 맞도록 채널 설정
		Sphere,
		Params
	);

	// 4. 무언가 맞았다면?
	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();

		// 맞은 대상이 유효하고, 생존자 태그를 가지고 있다면 (태그는 언리얼 에디터에서 설정)
		if (HitActor && HitActor->ActorHasTag(TEXT("Hider")))
		{
			// 로그를 띄웁니다!
			SCREEN_LOG(TEXT("Survivor Hit!"), FColor::Red);

			// 나중에 여기에 생존자의 체력을 깎는 코드를 넣으면 됩니다.
			// UGameplayStatics::ApplyDamage(HitActor, 10.0f, GetController(), this, UDamageType::StaticClass());
		}
		else
		{
			SCREEN_LOG(TEXT("Hit something else"), FColor::White);
		}
	}

	// =========================================================
	// [테스트용] 에디터에서 내 공격 범위가 어떻게 생겼는지 눈으로 보기 위해 구체를 그립니다.
	// (개발 끝나면 지우시면 됩니다)
	FColor DrawColor = bHit ? FColor::Red : FColor::Green;
	DrawDebugCapsule(GetWorld(), Start + (ForwardVector * (AttackRange * 0.5f)), AttackRange * 0.5f, AttackRadius, ForwardVector.Rotation().Quaternion(), DrawColor, false, 2.0f);
	// =========================================================
}

void ASeekerCharacter::FireChargeRifle()
{
	if (!RevolverMesh) return;

	// 시작점과 카메라 방향 구하기
	FVector Start = RevolverMesh->GetSocketLocation(FName("Muzzle_Socket"));
	FVector ForwardDir;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// 멀티플레이에서는 클라이언트의 카메라 회전값을 정확히 가져오기 위해 ControlRotation을 씁니다.
		ForwardDir = PC->GetControlRotation().Vector();
	}
	else
	{
		ForwardDir = GetActorForwardVector();
	}

	// 시작 위치 미세 조정
	FVector RightDir = FRotationMatrix(ForwardDir.Rotation()).GetScaledAxis(EAxis::Y);
	FVector UpDir = FRotationMatrix(ForwardDir.Rotation()).GetScaledAxis(EAxis::Z);
	Start += (ForwardDir * LaserStartOffset.X) + (RightDir * LaserStartOffset.Y) + (UpDir * LaserStartOffset.Z);

	Server_FireChargeRifle(Start, ForwardDir);
}

void ASeekerCharacter::Server_FireChargeRifle_Implementation(FVector TraceStart, FVector ShootDir)
{
	float InfiniteRange = 100000.0f;
	FVector End = TraceStart + (ShootDir * InfiniteRange);

	// 끝 위치 미세 조정
	FVector RightDir = FRotationMatrix(ShootDir.Rotation()).GetScaledAxis(EAxis::Y);
	FVector UpDir = FRotationMatrix(ShootDir.Rotation()).GetScaledAxis(EAxis::Z);
	End += (ShootDir * LaserEndOffset.X) + (RightDir * LaserEndOffset.Y) + (UpDir * LaserEndOffset.Z);

	FCollisionShape LaserShape = FCollisionShape::MakeSphere(RifleRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	TArray<FHitResult> HitResults;

	// ========================================================
	// [핵심 변경점] Channel 대신 Object Type으로 검사합니다.
	// ========================================================
	FCollisionObjectQueryParams ObjectQueryParams;
	// 벽(WorldStatic)은 무시하고, 오직 캐릭터(Pawn) 오브젝트 타입만 뚫고 가며 찾습니다!
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	bool bHit = GetWorld()->SweepMultiByObjectType(
		HitResults,
		TraceStart,
		End,
		FQuat::Identity,
		ObjectQueryParams, // <-- 여기가 변경됨
		LaserShape,
		Params
	);

	if (bHit)
	{
		TSet<AActor*> DamagedActors;

		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();

			if (HitActor && HitActor->ActorHasTag(TEXT("Hider")) && !DamagedActors.Contains(HitActor))
			{
				DamagedActors.Add(HitActor);

				SCREEN_LOG(TEXT("Survivor Hit with Piercing Laser!"), FColor::Red);

				// 데미지 처리
				// UGameplayStatics::ApplyDamage(HitActor, 10.0f, GetController(), this, UDamageType::StaticClass());
			}
		}
	}

	Multicast_DrawLaserBeam(TraceStart, End);
}

// ========================================================
// 단계 3: 모두에게 그리기 (각자의 모니터에서 개별적으로 원통 스폰!)
// ========================================================
void ASeekerCharacter::Multicast_DrawLaserBeam_Implementation(FVector Start, FVector End)
{
	if (LaserActorClass)
	{
		// 서버와 클라이언트 모두 각자의 컴퓨터에서 스스로 원통을 하나씩 만듭니다.
		FVector CenterPosition = (Start + End) * 0.5f;
		FRotator LaserRotation = FRotationMatrix::MakeFromZ(End - Start).Rotator();

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* SpawnedLaser = GetWorld()->SpawnActor<AActor>(LaserActorClass, CenterPosition, LaserRotation, SpawnParams);

		if (SpawnedLaser)
		{
			float ScaleXY = RifleRadius / 50.0f;
			float ScaleZ = (End - Start).Size() / 100.0f;
			SpawnedLaser->SetActorScale3D(FVector(ScaleXY, ScaleXY, ScaleZ));
			SpawnedLaser->SetLifeSpan(LaserLifeTime);
		}
	}
}

// =======================================================
// 기존 함수들 (기절, 볼팅, 판자 파괴)
// =======================================================

void ASeekerCharacter::Server_ApplyStun_Implementation(float StunTime)
{
	if (StunTime <= 0.0f) return;
	bIsStunned = true;
	OnRep_IsStunned();
	GetWorldTimerManager().SetTimer(StunTimerHandle, this, &ASeekerCharacter::EndStun, StunTime, false);
}

void ASeekerCharacter::EndStun()
{
	bIsStunned = false;
	OnRep_IsStunned();
}

void ASeekerCharacter::SetForceRotation(bool bIsSetRotation, FRotator targetRotation)
{
	_bIsSetRotation = bIsSetRotation;
	_targetRotation = targetRotation;
}

void ASeekerCharacter::StartDestroyPallet()
{
	if (IsLocallyControlled() || HasAuthority())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController())) DisableInput(PC);
	}
}

void ASeekerCharacter::StopDestroyPallet()
{
	if (IsLocallyControlled() || HasAuthority())
	{
		SetForceRotation(false);
		if (APlayerController* PC = Cast<APlayerController>(GetController())) EnableInput(PC);
	}
}

void ASeekerCharacter::ActivateAuraReveal()
{
	// 스킬이 쿨타임 중이거나, 내 화면(로컬 플레이어)이 아니면 실행 안 함
	if (!bCanUseAura || !IsLocallyControlled()) return;

	bCanUseAura = false;
	SCREEN_LOG(TEXT("Aura Skill Activated! (Duration: 3s)"), FColor::Purple);

	// 1. 맵에 있는 'Hider' 태그를 가진 모든 생존자를 찾습니다.
	TArray<AActor*> Hider;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("Hider"), Hider);

	for (AActor* HiderActor : Hider)
	{
		ACharacter* HiderChar = Cast<ACharacter>(HiderActor);
		if (HiderChar && HiderChar->GetMesh())
		{
			// [핵심] 내 화면에서만 생존자 메시의 '커스텀 뎁스'를 켭니다 (벽 투시 활성화)
			HiderChar->GetMesh()->SetRenderCustomDepth(true);
			HiderChar->GetMesh()->SetCustomDepthStencilValue(1); // 1번 색상 지정 (보통 빨간색으로 설정)
		}
	}

	// 2. 3초 뒤에 투시를 끄는 타이머 작동
	GetWorldTimerManager().SetTimer(AuraDurationTimer, this, &ASeekerCharacter::EndAuraReveal, AuraDuration, false);

	// 3. 10초 뒤에 쿨타임을 초기화하는 타이머 작동
	GetWorldTimerManager().SetTimer(AuraCooldownTimer, this, &ASeekerCharacter::ResetAuraCooldown, AuraCooldown, false);
}

void ASeekerCharacter::EndAuraReveal()
{
	TArray<AActor*> Hiders;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("Hider"), Hiders);

	for (AActor* HiderActor : Hiders)
	{
		ACharacter* HiderChar = Cast<ACharacter>(HiderActor);
		if (HiderChar && HiderChar->GetMesh())
		{
			// [수정] 렌더링 끄기 + 스텐실 번호를 0으로 강제 초기화!
			HiderChar->GetMesh()->SetRenderCustomDepth(false);
			HiderChar->GetMesh()->SetCustomDepthStencilValue(0);
		}
	}
	SCREEN_LOG(TEXT("Aura Vision Ended."), FColor::White); // 이 로그가 뜨는지 꼭 확인하세요!
}

void ASeekerCharacter::ResetAuraCooldown()
{
	// 10초가 지나면 다시 스킬을 쓸 수 있게 해줍니다.
	bCanUseAura = true;
	SCREEN_LOG(TEXT("Aura Skill Ready!"), FColor::Green);
}

// =======================================================
// 돌진(Dash) 스킬 로직
// =======================================================

void ASeekerCharacter::OnRep_IsDashing()
{
	// 다른 사람(생존자) 모니터에서 bIsDashing이 true로 바뀌면 실행!
	if (bIsDashing)
	{
		GetMesh()->GlobalAnimRateScale = 4.0f;
		DynamicBodyMaterial->SetVectorParameterValue(TEXT("BodyTint"), FLinearColor(2.0f, 0.1f, 0.1f)); // 약간 빛나는 빨강
	}
	// 돌진이 끝나서 false로 바뀌면 실행!
	else
	{
		GetMesh()->GlobalAnimRateScale = 1.0f;
		DynamicBodyMaterial->SetVectorParameterValue(TEXT("BodyTint"), FLinearColor::White);
	}
}

void ASeekerCharacter::StartDash()
{
	// ========================================================
	// [추가 1] 근접 무기(Melee)를 들고 있지 않다면 돌진 불가!
	// ========================================================
	if (CurrentWeapon != EWeaponType::Melee) return;

	// 기존 예외 처리 (기절, 이미 돌진 중, 공격 중일 때 막기)
	if (bIsStunned || bIsDashing || bIsCharging || bIsMeleeCharging) return;

	bIsDashing = true;
	DashDirection = GetActorForwardVector();
	SetForceRotation(true, DashDirection.Rotation());

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetIgnoreLookInput(true); // 마우스 시야 잠금

		// ========================================================
		// [추가 2] 플레이어의 WASD(이동) 입력을 완벽하게 무시!
		// ========================================================
		//PC->SetIgnoreMoveInput(true);
	}

	GetCharacterMovement()->MaxWalkSpeed = DashSpeed;
	GetCharacterMovement()->MaxAcceleration = 100000.0f;

	OnRep_IsDashing();
	Server_StartDash(DashDirection);
	SCREEN_LOG(TEXT("Dash Started!"), FColor::Yellow);
}

void ASeekerCharacter::Server_StartDash_Implementation(FVector Dir)
{
	bIsDashing = true;
	DashDirection = Dir;
	SetForceRotation(true, Dir.Rotation());
	GetCharacterMovement()->MaxWalkSpeed = DashSpeed;
	GetCharacterMovement()->MaxAcceleration = 100000.0f;

	// [수정] 서버도 호출!
	OnRep_IsDashing();
}

void ASeekerCharacter::StopDash()
{
	if (!bIsDashing) return;

	bIsDashing = false;
	SetForceRotation(false, FRotator::ZeroRotator);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetIgnoreLookInput(false); // 마우스 시야 잠금 해제

		// ========================================================
		// [추가 3] 돌진이 끝났으니 다시 WASD(이동) 입력 허용!
		// ========================================================
		PC->SetIgnoreMoveInput(false);
	}

	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	GetCharacterMovement()->MaxAcceleration = DefaultAcceleration;

	OnRep_IsDashing();
	Server_StopDash();
	SCREEN_LOG(TEXT("Dash Stopped! (Stunned for 3s)"), FColor::White);
}

void ASeekerCharacter::Server_StopDash_Implementation()
{
	bIsDashing = false;
	SetForceRotation(false, FRotator::ZeroRotator);
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	GetCharacterMovement()->MaxAcceleration = DefaultAcceleration;

	// [수정] 서버도 복구!
	OnRep_IsDashing();

	Server_ApplyStun(DashStunTime);
}