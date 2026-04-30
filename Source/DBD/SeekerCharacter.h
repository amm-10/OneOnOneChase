#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SeekerCharacter.generated.h"

// 무기 종류 정의
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Melee   UMETA(DisplayName = "Melee Weapon"),
	Ranged  UMETA(DisplayName = "Charge Rifle")
};

UCLASS()
class DBD_API ASeekerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASeekerCharacter();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DefaultWalkSpeed;

	// 돌진 후 원래 가속도로 되돌리기 위한 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DefaultAcceleration;

	// ==========================================
	// 기존 변수 (볼팅 & 기절)
	// ==========================================
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vaulting")
	bool _bIsSetRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vaulting")
	FRotator _targetRotation;

	UPROPERTY(ReplicatedUsing = OnRep_IsStunned, BlueprintReadOnly, Category = "Stun")
	bool bIsStunned;

	UFUNCTION()
	void OnRep_IsStunned();

	UFUNCTION(BlueprintImplementableEvent, Category = "Stun")
	void BP_OnStunStateChanged(bool bStunned);

	FTimerHandle StunTimerHandle;

	// ==========================================
	// 공통 무기 변수
	// ==========================================
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EWeaponType CurrentWeapon;

	UFUNCTION()
	void OnRep_CurrentWeapon();

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void BP_ChangeWeaponVisibility();

	// ==========================================
	// 총(Ranged) 차징 관련 변수
	// ==========================================
	UPROPERTY()
	USkeletalMeshComponent* RevolverMesh;

	// 레이저 발사 시작점 미세 조정 (X: 앞뒤, Y: 좌우, Z: 상하)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ranged")
	FVector LaserStartOffset;

	// 레이저 끝점 미세 조정 (X: 앞뒤, Y: 좌우, Z: 상하)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ranged")
	FVector LaserEndOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged")
	bool bIsCharging;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ranged")
	float ChargeTime;

	// 총알(원통)의 두께(반지름) (예: 10cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ranged")
	float RifleRadius;

	// 블루프린트에서 만든 레이저 원통 액터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ranged")
	TSubclassOf<AActor> LaserActorClass;

	// 레이저가 화면에 남아있는 시간 (디버그랑 똑같이 2초로 기본값 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ranged")
	float LaserLifeTime = 2.0f;

	FTimerHandle ChargeTimerHandle;

	// ==========================================
	// 근접(Melee) 런지 공격 관련 변수
	// ==========================================
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadOnly, Category = "Weapon|Melee")
	bool bIsMeleeCharging;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Melee")
	float MaxMeleeChargeTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Melee")
	float LungeWalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Melee")
	float MeleeAttackStunTime;

	// 블루프린트에서 설정할 근접 공격 몽타주 에셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Melee")
	UAnimMontage* MeleeAttackMontage;

	FTimerHandle MeleeChargeTimerHandle;


	// ==========================================
	// 오라 스킬 관련 변수
	// ==========================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Aura")
	float AuraDuration = 3.0f; // 3초간 보임

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Aura")
	float AuraCooldown = 10.0f; // 10초 쿨타임

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability|Aura")
	bool bCanUseAura = true; // 스킬 사용 가능 여부

	FTimerHandle AuraDurationTimer;
	FTimerHandle AuraCooldownTimer;

	// ==========================================
	// 돌진 관련 변수
	// ==========================================
	UPROPERTY(ReplicatedUsing = OnRep_IsDashing, VisibleAnywhere, BlueprintReadOnly, Category = "Ability|Dash")
	bool bIsDashing = false;
	UFUNCTION()
	void OnRep_IsDashing();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Dash")
	float DashSpeed = 1500.0f; // 돌진 속도

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Dash")
	float DashStunTime = 3.0f; // 돌진 종료 후 기절 시간

	// 돌진 시작 시 바라보던 방향을 기억할 변수
	FVector DashDirection;
	
	// 동적으로 색상을 바꿀 머티리얼 인스턴스
	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicBodyMaterial;


public:
	// ==========================================
	// 블루프린트 연출용 이벤트 모음
	// ==========================================
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Weapon|Events")
	void BP_ShowMeleeWeapon();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Weapon|Events")
	void BP_HideMeleeWeapon();

	//UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Events")
	//void BP_OnMeleeChargeStart();

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Events")
	void BP_OnChargeStart();

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Events")
	void BP_OnChargeCancel();

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Events")
	void BP_OnFire();

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Events")
	void BP_OnMeleeAttack();

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ==========================================
	// 무기 제어 및 몽타주 함수
	// ==========================================
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Weapon")
	void EquipMelee();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Weapon")
	void EquipRanged();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Ranged")
	void ExecuteFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Melee")
	void ExecuteMeleeAttack();

	// 애니메이션 노티파이에서 호출할 타격 판정 함수
	UFUNCTION(BlueprintCallable, Category = "Weapon|Events")
	void CheckMeleeHit();

	// Ranged 차징 완료 후 실제 총알(빔)을 쏘는 함수
	UFUNCTION(BlueprintCallable, Category = "Weapon|Events")
	void FireChargeRifle();

protected:
	UFUNCTION(Server, Reliable)
	void Server_FireChargeRifle(FVector TraceStart, FVector ShootDir);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_DrawLaserBeam(FVector Start, FVector End);

public:
	// 노티파이 및 공격 종료 시 호출할 애니메이션 정지/재생 함수
	UFUNCTION(BlueprintCallable, Category = "Weapon|Events")
	void PauseMeleeMontage();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Events")
	void ResumeMeleeMontage();

	// 서버 동기화 함수
	UFUNCTION(Server, Reliable)
	void Server_SyncMeleeCharge(bool bCharging);

	// ==========================================
	// 기존 함수 (볼팅 & 판자 파괴)
	// ==========================================
	UFUNCTION(BlueprintCallable, Category = "Vaulting")
	void SetForceRotation(bool bIsSetRotation, FRotator targetRotation = FRotator::ZeroRotator);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stun")
	void Server_ApplyStun(float StunTime);

	void EndStun();

	UFUNCTION(BlueprintCallable, Category = "Vaulting")
	void StartDestroyPallet();

	UFUNCTION(BlueprintCallable, Category = "Vaulting")
	void StopDestroyPallet();

	// 오라 스킬 관련 함수
	// 블루프린트에서 숫자 '3'을 누르면 호출할 함수
	UFUNCTION(BlueprintCallable, Category = "Ability|Aura")
	void ActivateAuraReveal();

	// 돌진 관련 함수
	UFUNCTION(BlueprintCallable, Category = "Ability|Dash")
	void StartDash();

	UFUNCTION(BlueprintCallable, Category = "Ability|Dash")
	void StopDash();

protected:
	// 오라 관련
	// 내부적으로 타이머가 끝나면 실행될 함수들
	void EndAuraReveal();
	void ResetAuraCooldown();

	// 돌진 관련
	// 서버 동기화용 RPC
	UFUNCTION(Server, Reliable)
	void Server_StartDash(FVector Dir);

	UFUNCTION(Server, Reliable)
	void Server_StopDash();
};