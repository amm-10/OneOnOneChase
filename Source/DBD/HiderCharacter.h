// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HiderCharacter.generated.h"
// Net/UnrealNetwork.h 는 cpp 파일에 포함하는 것이 빌드 속도에 더 좋습니다.

UCLASS()
class DBD_API AHiderCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AHiderCharacter();

	// [추가] 네트워크 동기화를 위해 반드시 필요한 엔진 기본 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DefaultWalkSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vaulting")
	bool _bIsSetRotation;

	// 고정시킬 목표 회전값
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vaulting")
	FRotator _targetRotation;

	// ========================================================
	// [수정] 애니메이션(상호작용) 중인지 체크하는 변수 (동기화 설정!)
	// 값이 변하면 OnRep_IsInteracting 함수를 자동으로 실행합니다.
	// ========================================================
	UPROPERTY(ReplicatedUsing = OnRep_IsInteracting, BlueprintReadWrite, Category = "State")
	bool bIsInteracting = false;

	// [추가] 상태가 변할 때 콜리전을 제어해 줄 알림 함수
	UFUNCTION()
	void OnRep_IsInteracting();

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

	// 단차 오르기 시작할 때 호출할 함수 (충돌 끄기, 입력 막기, 회전 고정)
	UFUNCTION(BlueprintCallable, Category = "Climb")
	void StartClimb();
	// 단차 오르기가 끝났을 때 호출할 함수 (모두 원상복구)
	UFUNCTION(BlueprintCallable, Category = "Climb")
	void StopClimb();
};