// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SeekerCharacter.generated.h"

UCLASS()
class DBD_API ASeekerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASeekerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vaulting")
	bool _bIsSetRotation;

	// 고정시킬 목표 회전값
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vaulting")
	FRotator _targetRotation;

	// 기절 상태 동기화 변수
	UPROPERTY(ReplicatedUsing = OnRep_IsStunned, BlueprintReadOnly, Category = "Stun")
	bool bIsStunned;

	// 변수 동기화 시 클라이언트에서 자동 호출될 함수
	UFUNCTION()
	void OnRep_IsStunned();

	// 블루프린트에서 이펙트를 켜고 끌 수 있게 던져주는 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Stun")
	void BP_OnStunStateChanged(bool bStunned);

	// Delay 노드를 대신할 타이머 핸들
	FTimerHandle StunTimerHandle;



public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	// 블루프린트에서 회전값과 상태를 설정할 수 있는 함수
	UFUNCTION(BlueprintCallable, Category = "Vaulting")
	void SetForceRotation(bool bIsSetRotation, FRotator targetRotation = FRotator::ZeroRotator);


	// 서버에서 실행될 기절 시작 함수
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Stun")
	void Server_ApplyStun(float StunTime);

	// 타이머가 끝나면 호출될 기절 종료 함수
	void EndStun();


	// 판자 부수기 시작할 때 호출할 함수 (충돌 끄기, 입력 막기, 회전 고정)
	UFUNCTION(BlueprintCallable, Category = "Vaulting")
	void StartDestroyPallet();
	// 판자 부수기가 끝났을 때 호출할 함수
	UFUNCTION(BlueprintCallable, Category = "Vaulting")
	void StopDestroyPallet();
};
