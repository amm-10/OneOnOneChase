#include "Pallet.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Engine/Engine.h" // 화면에 텍스트를 띄우기 위해 추가

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

// 모든 클라이언트(플레이어)의 화면에 텍스트를 그려줍니다.
void APallet::Multicast_PrintAction_Implementation(const FString& ActionText)
{
	if (GEngine)
	{
		// -1: 기존 메시지 덮어쓰지 않음, 3.f: 3초 동안 표시, FColor::Cyan: 민트색 글씨
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, ActionText);
	}
}

void APallet::Server_Interact_Implementation(AActor* Interactor)
{
	if (!Interactor) return;

	bool bIsHider = Interactor->ActorHasTag("Hider");
	bool bIsSeeker = Interactor->ActorHasTag("Seeker");

	// 1. 판자가 세워져 있을 때 (판자 내리기)
	if (PalletState == EPalletState::Upright && bIsHider)
	{
		if (bIsHider)
		{
			if (FrontInteractionBox->IsOverlappingActor(Interactor))
			{
				PalletState = EPalletState::DroppedForward;
				Multicast_PrintAction(TEXT("Front Drop"));
			}
			else if (BackInteractionBox->IsOverlappingActor(Interactor))
			{
				PalletState = EPalletState::DroppedBackward;
				Multicast_PrintAction(TEXT("Back Drop"));
			}
			else
			{
				return;
			}
		}

		OnRep_PalletState();

		TArray<AActor*> OverlappingActors;
		StunBox->GetOverlappingActors(OverlappingActors);

		for (AActor* Actor : OverlappingActors)
		{
			if (Actor->ActorHasTag("Seeker"))
			{
				Multicast_PrintAction(TEXT("Seeker Stun!"));
			}
		}
	}
	// 2. 판자가 이미 내려져 있을 때 (넘어가기 or 부수기)
	else if (PalletState == EPalletState::DroppedForward || PalletState == EPalletState::DroppedBackward)
	{
		if (bIsHider)
		{
			// 생존자가 앞/뒤 어느 쪽에서 넘어가는지 판별
			if (FrontInteractionBox->IsOverlappingActor(Interactor))
			{
				Multicast_PrintAction(TEXT("Front Slide"));
			}
			else if (BackInteractionBox->IsOverlappingActor(Interactor))
			{
				Multicast_PrintAction(TEXT("Back Slide"));
			}
		}
		else if (bIsSeeker)
		{
			PalletState = EPalletState::Destroyed;
			Multicast_PrintAction(TEXT("Pallet Destroyed!"));
			OnRep_PalletState();
		}
	}
}

void APallet::OnRep_PalletState()
{
	if (PalletState == EPalletState::DroppedForward)
	{
		BP_OnPalletDropped(true);
	}
	else if (PalletState == EPalletState::DroppedBackward)
	{
		BP_OnPalletDropped(false);
	}
	else if (PalletState == EPalletState::Destroyed)
	{
		BP_OnPalletDestroyed();
		SetActorEnableCollision(false);
		SetActorHiddenInGame(true);
	}
}