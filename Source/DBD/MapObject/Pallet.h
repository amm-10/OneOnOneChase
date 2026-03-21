#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pallet.generated.h"

UENUM(BlueprintType)
enum class EPalletState : uint8
{
	Upright,    // 0: 세워짐
	Dropped,    // 1: 넘어짐 (앞/뒤 구분 제거)
	Destroyed   // 2: 부서짐 
};

UCLASS()
class DBD_API APallet : public AActor
{
	GENERATED_BODY()

public:
	APallet();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* PalletMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* FrontInteractionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* BackInteractionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* StunBox;

	UPROPERTY(ReplicatedUsing = OnRep_PalletState, BlueprintReadOnly, Category = "State")
	EPalletState PalletState;

	UFUNCTION()
	void OnRep_PalletState();

	// 변경됨: 방향 구분이 필요 없으므로 매개변수(bool bIsForward)를 제거했습니다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void BP_OnPalletDropped();

	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void BP_OnPalletDestroyed();

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void BP_OnSeekerStunned(AActor* Seeker, float StunTime);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void BP_OnHiderVault(AActor* Hider);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void BP_OnSeekerDestroyPallet(AActor* Seeker);

public:
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void DoInteract(AActor* Interactor);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PrintAction(const FString& ActionText);
};