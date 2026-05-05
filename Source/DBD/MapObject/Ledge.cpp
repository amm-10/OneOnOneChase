// Fill out your copyright notice in the Description page of Project Settings.
#include "Ledge.h"
#include "Components/BoxComponent.h" 

ALedge::ALedge()
{
	bReplicates = true;

	USceneComponent* DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
	RootComponent = DefaultRoot;

	LedgeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LedgeMesh"));
	LedgeMesh->SetupAttachment(RootComponent);
}

void ALedge::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ALedge::BeginPlay()
{
	Super::BeginPlay();
	GetComponents<UBoxComponent>(InteractionBoxes);

	// ========================================================
	// [추가] Ledge 메쉬의 크기를 측정해서 오프셋 자동 계산!
	// ========================================================
	if (LedgeMesh)
	{
		// 1. 메쉬의 순수 로컬 영역(Bounds) 가져오기
		FVector Min, Max;
		LedgeMesh->GetLocalBounds(Min, Max);

		// 2. 에디터에서 조절한 스케일(크기 배율)을 곱해 실제 월드에서의 크기(길이) 구하기
		FVector MeshSize = (Max - Min) * LedgeMesh->GetComponentScale();

		// 3. 유저님의 공식 적용 (스케일이 음수일 때를 대비해 절대값 처리)
		// X: Y축 크기의 3분의 1
		// Z: Z축 전체 크기
		ArriveLocationOffset.X = FMath::Abs(MeshSize.Y) / 3.0f;
		ArriveLocationOffset.Y = 0.0f; // 좌우 치우침 없음
		ArriveLocationOffset.Z = FMath::Abs(MeshSize.Z);
	}
}

void ALedge::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALedge::DoInteract(AActor* Interactor)
{
	if (!Interactor || !HasAuthority()) return;

	bool bIsHider = Interactor->ActorHasTag("Hider");
	bool bIsSeeker = Interactor->ActorHasTag("Seeker");

	UBoxComponent* ActiveBox = nullptr;
	for (UBoxComponent* Box : InteractionBoxes)
	{
		if (Box->IsOverlappingActor(Interactor))
		{
			ActiveBox = Box;
			break;
		}
	}

	if (!ActiveBox) return;

	FVector SnapLocation = ActiveBox->GetComponentLocation();
	SnapLocation.Z = Interactor->GetActorLocation().Z;

	// [픽스 적용] 루트가 아닌 '메쉬'를 바라보게 계산
	FVector DirectionToLedge = LedgeMesh->GetComponentLocation() - SnapLocation;
	DirectionToLedge.Z = 0.0f;
	FRotator SnapRotation = DirectionToLedge.Rotation();

	Interactor->SetActorLocationAndRotation(SnapLocation, SnapRotation, false, nullptr, ETeleportType::TeleportPhysics);

	// [픽스 적용] 강제로 북쪽을 바라보는 현상 방지 (컨트롤러 시야도 같이 돌려줌)
	if (APawn* InteractorPawn = Cast<APawn>(Interactor))
	{
		if (AController* PC = InteractorPawn->GetController())
		{
			PC->SetControlRotation(SnapRotation);
		}
	}

	if (bIsHider)
	{
		FVector StartLoc = ActiveBox->GetComponentLocation();
		StartLoc.Z = Interactor->GetActorLocation().Z;

		// 회전된 방향으로 우리가 방금 '자동 계산한 오프셋' 적용!
		FVector WorldOffset = SnapRotation.RotateVector(ArriveLocationOffset);
		FVector EndLoc = StartLoc + WorldOffset;

		BP_OnHiderClimb(Interactor, StartLoc, EndLoc);
	}
}

FTransform ALedge::GetSnapTransform(AActor* Interactor)
{
	if (!Interactor || InteractionBoxes.IsEmpty()) return FTransform();

	UBoxComponent* ActiveBox = nullptr;
	float MinDistance = MAX_flt;

	for (UBoxComponent* Box : InteractionBoxes)
	{
		float Dist = FVector::DistSquared(Interactor->GetActorLocation(), Box->GetComponentLocation());
		if (Dist < MinDistance)
		{
			MinDistance = Dist;
			ActiveBox = Box;
		}
	}

	if (!ActiveBox) return FTransform();

	FVector SnapLocation = ActiveBox->GetComponentLocation();
	SnapLocation.Z = Interactor->GetActorLocation().Z;

	// [픽스 적용] 루트가 아닌 '메쉬'를 바라보게 계산
	FVector DirectionToLedge = LedgeMesh->GetComponentLocation() - SnapLocation;
	DirectionToLedge.Z = 0.0f;
	FRotator SnapRotation = DirectionToLedge.Rotation();

	return FTransform(SnapRotation, SnapLocation);
}