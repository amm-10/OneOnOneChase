#include "DashPad.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"

ADashPad::ADashPad()
{
    PrimaryActorTick.bCanEverTick = false;

    // 1. 컴포넌트 조립
    PadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PadMesh"));
    RootComponent = PadMesh;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetupAttachment(RootComponent);

    InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    InteractionBox->SetupAttachment(RootComponent);

    DirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("DirectionArrow"));
    DirectionArrow->SetupAttachment(RootComponent);

    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ADashPad::OnPadOverlap);

    // 2. 변수 기본값 초기화
    bIsActive = false;
    bIsExpired = false;
    HiderSpeed = 2000.0f;
    SeekerSpeed = 3000.0f;
    DisableTime = 1.0f;
}

void ADashPad::OnPadOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 1. 발판이 꺼져있거나 대상이 없으면 무시
    if (!bIsActive || !OtherActor) return;

    // 2. 닿은 액터가 캐릭터인지 확인
    ACharacter* TargetCharacter = Cast<ACharacter>(OtherActor);
    if (TargetCharacter)
    {
        // 3. 태그를 확인하여 속도 결정
        float FinalSpeed = 0.0f;
        if (TargetCharacter->ActorHasTag("Hider"))
        {
            FinalSpeed = HiderSpeed;
        }
        else if (TargetCharacter->ActorHasTag("Seeker"))
        {
            FinalSpeed = SeekerSpeed;
        }
        else
        {
            return; // Hider나 Seeker가 아니면 발사하지 않음
        }

        // 4. 발사 벡터 계산 (앞방향 * 속도)
        FVector LaunchVelocity = DirectionArrow->GetForwardVector() * FinalSpeed;

        // Z축(위쪽)으로 200만큼 더해서 바닥 마찰력을 없앰
        LaunchVelocity.Z += 200.0f;

        // 5. 캐릭터 발사! (XY Override = true, Z Override = true)
        TargetCharacter->LaunchCharacter(LaunchVelocity, true, true);

        // 6. 조작 불능 처리 및 복구 타이머
        APlayerController* PC = Cast<APlayerController>(TargetCharacter->GetController());
        if (PC)
        {
            TargetCharacter->DisableInput(PC);

            // DisableTime 변수 시간만큼 지난 후 EnableInput을 실행하는 람다(Lambda) 타이머
            FTimerHandle RestoreInputTimer;
            GetWorldTimerManager().SetTimer(RestoreInputTimer, [TargetCharacter, PC]()
                {
                    // 시간이 지났을 때 캐릭터와 컨트롤러가 유효한지(파괴되지 않았는지) 확인 후 조작 복구
                    if (TargetCharacter && PC)
                    {
                        TargetCharacter->EnableInput(PC);
                    }
                }, DisableTime, false);
        }
    }
}

void ADashPad::DoInteract(AActor* Interactor)
{
    if (!Interactor || !HasAuthority()) return;
    
    bool bIsHider = Interactor->ActorHasTag("Hider");
	if (!bIsHider || bIsActive || bIsExpired) return; 

    BP_OnHiderInteractDashPad(Interactor);
}