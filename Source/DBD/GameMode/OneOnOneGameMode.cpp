#include "OneOnOneGameMode.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"


AOneOnOneGameMode::AOneOnOneGameMode()
{
	DefaultPawnClass = nullptr;
}

void AOneOnOneGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 PlayerCount = GetNumPlayers();

	APawn* SpawnedPawn = nullptr;
	FVector SpawnLocation = FVector(900.0f, 1200.0f, 100.0f);
	FRotator SpawnRotation = FRotator::ZeroRotator;

	if (PlayerCount == 1 && SeekerClass != nullptr)
	{
		// 1. 스폰 규칙 설정: 충돌해도 무조건 스폰(AlwaysSpawn)하게 만듭니다.
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 2. 맨 끝에 SpawnParams를 추가해서 스폰합니다.
		SpawnedPawn = GetWorld()->SpawnActor<APawn>(SeekerClass, SpawnLocation, SpawnRotation, SpawnParams);
	}
	else if (PlayerCount == 2 && HiderClass != nullptr)
	{
		// 1. 스폰 규칙 설정: 충돌해도 무조건 스폰(AlwaysSpawn)하게 만듭니다.
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 2. 맨 끝에 SpawnParams를 추가해서 스폰합니다.
		SpawnedPawn = GetWorld()->SpawnActor<APawn>(HiderClass, SpawnLocation, SpawnRotation, SpawnParams);
	}

	if (SpawnedPawn && NewPlayer)
	{
		NewPlayer->Possess(SpawnedPawn);
	}
}