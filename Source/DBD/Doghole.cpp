// Fill out your copyright notice in the Description page of Project Settings.

#include "Doghole.h"
#include "Components/BoxComponent.h" 


ADoghole::ADoghole()
{
	bReplicates = true;

	USceneComponent* DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
	RootComponent = DefaultRoot;

	DogholeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DogholeMesh"));
	DogholeMesh->SetupAttachment(RootComponent);

	FrontInteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("FrontInteractionBox"));
	FrontInteractionBox->SetupAttachment(RootComponent);

	BackInteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BackInteractionBox"));
	BackInteractionBox->SetupAttachment(RootComponent);
}

void ADoghole::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ADoghole::BeginPlay()
{
	Super::BeginPlay();

}

void ADoghole::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ADoghole::DoInteract(AActor* Interactor)
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

	// ★ 위치와 회전 강제 적용 (TeleportPhysics 옵션 사용)
	Interactor->SetActorLocationAndRotation(SnapLocation, SnapRotation, false, nullptr, ETeleportType::TeleportPhysics);

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
}


FTransform ADoghole::GetSnapTransform(AActor* Interactor)
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

