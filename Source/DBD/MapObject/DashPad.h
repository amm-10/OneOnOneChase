#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DashPad.generated.h"

UCLASS()
class DBD_API ADashPad : public AActor // <- YOURPROJECT 부분을 수정하세요!
{
    GENERATED_BODY()

public:
    ADashPad();

protected:
    // --- 1. 컴포넌트 선언 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* PadMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* TriggerBox;     // 밟으면 날아가는 박스

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* InteractionBox; // 상호작용 가능한 범위 박스

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UArrowComponent* DirectionArrow; // 날아갈 방향

    // --- 2. 블루프린트에서 사용할 데이터 변수 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash Settings")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash Settings")
    bool bIsExpired; // 한 번 사용 후 영구 비활성화 여부

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash Settings")
    float HiderSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash Settings")
    float SeekerSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash Settings")
    float DisableTime;

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void BP_OnHiderInteractDashPad(AActor* Hider);

    // 충돌 이벤트 함수 선언 (반드시 UFUNCTION 매크로가 필요합니다)
    UFUNCTION()
    void OnPadOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
public:
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void DoInteract(AActor* Interactor);
};