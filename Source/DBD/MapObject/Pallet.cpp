#include "Pallet.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
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


void APallet::OnRep_PalletState()
{
	// 변경됨: 단일 상태(Dropped)로 통합
	if (PalletState == EPalletState::Dropped)
	{
		// 1. 일단 모든 채널을 Block으로 막습니다.
		StunBox->SetCollisionResponseToAllChannels(ECR_Block);

		// 2. 카메라 채널만 다시 유령 취급(Ignore)으로 뚫어줍니다. (카메라 줌인 방지)
		StunBox->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

		BP_OnPalletDropped(); // 매개변수 없이 호출
	}
	else if (PalletState == EPalletState::Destroyed)
	{
		SetActorEnableCollision(false);
		SetActorHiddenInGame(true);
	}
}

void APallet::DoInteract(AActor* Interactor)
{
	if (!Interactor || !HasAuthority()) return;

	bool bIsHider = Interactor->ActorHasTag("Hider");
	bool bIsSeeker = Interactor->ActorHasTag("Seeker");

	// 1. 어떤 상호작용 박스에 닿아있는지 먼저 판별
	UBoxComponent* ActiveBox = nullptr;
	if (FrontInteractionBox->IsOverlappingActor(Interactor))
	{
		ActiveBox = FrontInteractionBox;
	}
	else if (BackInteractionBox->IsOverlappingActor(Interactor))
	{
		ActiveBox = BackInteractionBox;
	}

	// 어느 박스에도 닿아있지 않으면 상호작용 취소
	if (!ActiveBox) return;

	// 2. 캐릭터 위치를 상호작용 박스 중앙으로 이동 (스냅)
	FVector SnapLocation = ActiveBox->GetComponentLocation();
	SnapLocation.Z = Interactor->GetActorLocation().Z;

	// 3. 캐릭터가 판자 중심을 바라보도록 회전값 계산
	FVector DirectionToPallet = GetActorLocation() - SnapLocation;
	DirectionToPallet.Z = 0.0f;
	FRotator SnapRotation = DirectionToPallet.Rotation();

	// [디버깅] 회전 적용 전 생존자의 현재 회전값 확인
	//FRotator BeforeRot = Interactor->GetActorRotation();
	//if (GEngine)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("BeforeRot: %s"), *BeforeRot.ToString()));
	//}
	//UE_LOG(LogTemp, Warning, TEXT("BeforeRot: %s"), *BeforeRot.ToString());

	// ★ 위치와 회전 강제 적용 (TeleportPhysics 옵션 사용)
	Interactor->SetActorLocationAndRotation(SnapLocation, SnapRotation, false, nullptr, ETeleportType::TeleportPhysics);

	//// [디버깅] 회전 적용 후 생존자의 회전값 확인
	//FRotator AfterRot = Interactor->GetActorRotation();
	//if (GEngine)
	//{
	//	// 여기서 AfterRot과 Target(SnapRotation)이 일치하는지 보는 게 핵심입니다.
	//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("AfterRot: %s (Target: %s)"), *AfterRot.ToString(), *SnapRotation.ToString()));
	//}
	//UE_LOG(LogTemp, Warning, TEXT("AfterRot: %s (Target: %s)"), *AfterRot.ToString(), *SnapRotation.ToString());


	// ==========================================================
	// 판자가 세워져 있을 때 (생존자만 내릴 수 있음)
	// ==========================================================
	if (PalletState == EPalletState::Upright && bIsHider)
	{
		PalletMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

		PalletState = EPalletState::Dropped;


		// 블루프린트에서 생존자 '판자 내리기' 애니메이션 재생을 위한 이벤트 호출
		BP_OnHiderDropPallet(Interactor);


		// 스턴 박스 내부 캐릭터 검사
		TArray<AActor*> OverlappingActors;
		StunBox->GetOverlappingActors(OverlappingActors);

		for (AActor* Actor : OverlappingActors)
		{
			ACharacter* OverlappingChar = Cast<ACharacter>(Actor);
			if (OverlappingChar)
			{
				// 1. 술래의 현재 월드 위치
				FVector SeekerWorldLoc = OverlappingChar->GetActorLocation();

				// 2. 판자 기준 로컬 좌표로 변환 (판자 중심이 0,0,0인 공간)
				FVector SeekerLocalLoc = ActorToWorld().InverseTransformPosition(SeekerWorldLoc);

				// 3. 앞/뒤 상호작용 박스의 로컬 좌표 가져오기
				FVector FrontLocalLoc = ActorToWorld().InverseTransformPosition(FrontInteractionBox->GetComponentLocation());
				FVector BackLocalLoc = ActorToWorld().InverseTransformPosition(BackInteractionBox->GetComponentLocation());

				// 4. 술래의 로컬 X값과 각 박스의 로컬 X값 거리 비교
				float DistToFrontX = FMath::Abs(SeekerLocalLoc.X - FrontLocalLoc.X);
				float DistToBackX = FMath::Abs(SeekerLocalLoc.X - BackLocalLoc.X);

				// 5. 더 가까운 쪽의 X값만 채택 (Y, Z는 술래의 로컬값 그대로 유지)
				if (DistToFrontX < DistToBackX)
				{
					SeekerLocalLoc.X = FrontLocalLoc.X;
				}
				else
				{
					SeekerLocalLoc.X = BackLocalLoc.X;
				}

				// 6. 다시 월드 좌표로 변환하여 텔레포트
				FVector TargetWorldLocation = ActorToWorld().TransformPosition(SeekerLocalLoc);

				OverlappingChar->SetActorLocation(TargetWorldLocation, false, nullptr, ETeleportType::TeleportPhysics);
				OverlappingChar->GetCharacterMovement()->StopMovementImmediately();
			}

			if (Actor->ActorHasTag("Seeker"))
			{
				BP_OnSeekerStunned(Actor, StunDuration);
			}
		}

		// 서버 수동 상태 업데이트
		OnRep_PalletState();
	}
	// ==========================================================
	// 판자가 이미 내려져 있을 때 (넘어가기 or 부수기)
	// ==========================================================
	else if (PalletState == EPalletState::Dropped)
	{
		if (bIsHider)
		{
			// 1. 반대쪽 상호작용 박스 판별
			UBoxComponent* TargetBox = (ActiveBox == FrontInteractionBox) ? BackInteractionBox : FrontInteractionBox;

			// 2. 시작 위치와 도착 위치 가져오기
			FVector StartLoc = ActiveBox->GetComponentLocation();
			FVector EndLoc = TargetBox->GetComponentLocation();

			// 3. Z축(높이)을 생존자의 현재 위치로 맞춰서 공중에 뜨거나 파묻히는 것 방지
			StartLoc.Z = Interactor->GetActorLocation().Z;
			EndLoc.Z = Interactor->GetActorLocation().Z;

			// 4. 도착 지점 연장: 넘어가는 방향으로 0.5미터(50유닛) 더 가기
			// 도착점에서 시작점을 빼서 넘어가는 방향 벡터를 구하고 정규화(길이를 1로 만듦)
			FVector VaultDirection = (EndLoc - StartLoc).GetSafeNormal();

			// 그 방향으로 50(cm)만큼 도착 지점을 더 밀어줌
			EndLoc += VaultDirection * 50.0f;

			// 5. 블루프린트로 이벤트 전달 (인자 포함)
			BP_OnHiderVault(Interactor, StartLoc, EndLoc);
		}
		else if (bIsSeeker)
		{
			// 블루프린트에서 술래 '부수기' 애니메이션 재생
			BP_OnSeekerDestroyPallet(Interactor);
		}
	}
}


FTransform APallet::GetSnapTransform(AActor* Interactor)
{
	if (!Interactor) return FTransform();

	// 1. 어떤 상호작용 박스에 가까운지 (또는 겹쳐있는지) 판별
	UBoxComponent* ActiveBox = nullptr;

	float DistToFront = FVector::DistSquared(Interactor->GetActorLocation(), FrontInteractionBox->GetComponentLocation());
	float DistToBack = FVector::DistSquared(Interactor->GetActorLocation(), BackInteractionBox->GetComponentLocation());

	// 겹쳐있지 않은 상태에서 스페이스바를 누를 수도 있으니, 단순 거리 비교로 찾는 것이 클라이언트 선판정에 더 안전합니다.
	ActiveBox = (DistToFront < DistToBack) ? FrontInteractionBox : BackInteractionBox;

	// 2. 위치 계산
	FVector SnapLocation = ActiveBox->GetComponentLocation();
	SnapLocation.Z = Interactor->GetActorLocation().Z;

	// 3. 회전 계산
	FVector DirectionToPallet = GetActorLocation() - SnapLocation;
	DirectionToPallet.Z = 0.0f;
	FRotator SnapRotation = DirectionToPallet.Rotation();

	// 계산된 위치와 회전값을 하나의 Transform으로 묶어서 반환
	return FTransform(SnapRotation, SnapLocation);
}