#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pallet.generated.h"

UENUM(BlueprintType)
enum class EPalletState : uint8
{
	Upright,          // 0: 세워짐
	DroppedForward,   // 1: 앞에서 상호작용 -> 뒤로 넘어짐
	DroppedBackward,  // 2: 뒤에서 상호작용 -> 앞으로 넘어짐
	Destroyed         // 3: 부서짐 
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

	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void BP_OnPalletDropped(bool bIsForward);

	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void BP_OnPalletDestroyed();

public:
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Interaction")
	void Server_Interact(AActor* Interactor);

	// 모든 플레이어의 화면에 텍스트를 띄워주는 멀티캐스트 함수
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PrintAction(const FString& ActionText);
};