#include "Pallet.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Engine/Engine.h"

APallet::APallet()
{
	bReplicates = true;

	USceneComponent* DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
	RootComponent = DefaultRoot;

	PalletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PalletMesh"));
	PalletMesh->SetupAttachment(RootComponent);

	FrontInteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("FrontInteractionBox"));
	FrontInteractionBox->SetupAttachment(RootComponent);

	BackInteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BackInteractionBox"));
	BackInteractionBox->SetupAttachment(RootComponent);

	StunBox = CreateDefaultSubobject<UBoxComponent>(TEXT("StunBox"));
	StunBox->SetupAttachment(RootComponent);

	PalletState = EPalletState::Upright;
}

void APallet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APallet, PalletState);
}

void APallet::Multicast_PrintAction_Implementation(const FString& ActionText)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, ActionText);
	}
}

void APallet::OnRep_PalletState()
{
	// 변경됨: 단일 상태(Dropped)로 통합
	if (PalletState == EPalletState::Dropped)
	{
		StunBox->SetCollisionResponseToAllChannels(ECR_Block);
		BP_OnPalletDropped(); // 매개변수 없이 호출
	}
	else if (PalletState == EPalletState::Destroyed)
	{
		BP_OnPalletDestroyed();
		SetActorEnableCollision(false);
		SetActorHiddenInGame(true);
	}
}

void APallet::DoInteract(AActor* Interactor)
{
	if (!Interactor || !HasAuthority()) return;

	bool bIsHider = Interactor->ActorHasTag("Hider");
	bool bIsSeeker = Interactor->ActorHasTag("Seeker");

	// 판자가 세워져 있을 때 (생존자만 내릴 수 있음)
	if (PalletState == EPalletState::Upright && bIsHider)
	{
		// 변경됨: 앞/뒤 상관없이 박스에 닿아있기만 하면 같은 Dropped 상태로 변경
		if (FrontInteractionBox->IsOverlappingActor(Interactor) || BackInteractionBox->IsOverlappingActor(Interactor))
		{
			PalletState = EPalletState::Dropped;
		}
		else return;

		// 서버 수동 상태 업데이트
		OnRep_PalletState();

		// 스턴 박스 내부 캐릭터 검사
		TArray<AActor*> OverlappingActors;
		StunBox->GetOverlappingActors(OverlappingActors);

		for (AActor* Actor : OverlappingActors)
		{
			// 캐릭터가 깔리지 않도록 바깥으로 밀어내기 (물리 엔진 강제 적용)
			ACharacter* OverlappingChar = Cast<ACharacter>(Actor);
			if (OverlappingChar)
			{
				// 1. 판자 중심에서 캐릭터를 향하는 방향 벡터
				FVector ToCharacter = OverlappingChar->GetActorLocation() - GetActorLocation();

				// 2. 판자의 로컬 X축 (앞쪽 방향) 가져오기 및 Z축 무시
				FVector PalletXAxis = GetActorForwardVector();
				PalletXAxis.Z = 0.0f;
				PalletXAxis.Normalize();

				// 3. 내적(Dot Product)을 계산하여 캐릭터의 위치 판별
				// 내적 값이 0 이상이면 캐릭터가 +X 방향에, 0 미만이면 -X 방향에 있음을 의미함
				float DotResult = FVector::DotProduct(ToCharacter, PalletXAxis);

				// 4. 가까운 쪽 X축 방향 결정 (+X 또는 -X)
				FVector PushDirection = (DotResult >= 0.0f) ? PalletXAxis : -PalletXAxis;

				// 5. 결정된 방향으로 캐릭터를 튕겨냄 (힘: 800)
				OverlappingChar->LaunchCharacter(PushDirection * 800.0f, true, true);
			}

			if (Actor->ActorHasTag("Seeker"))
			{
				BP_OnSeekerStunned(Actor, 3.0f);
			}
		}
	}
	// 판자가 이미 내려져 있을 때 (넘어가기 or 부수기)
	else if (PalletState == EPalletState::Dropped)
	{
		if (FrontInteractionBox->IsOverlappingActor(Interactor) || BackInteractionBox->IsOverlappingActor(Interactor))
		{
			if (bIsHider)
			{
				BP_OnHiderVault(Interactor);
			}
			else if (bIsSeeker)
			{
				BP_OnSeekerDestroyPallet(Interactor);
			}
		}
	}
}