#include "OneOnOneGameMode.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"


AOneOnOneGameMode::AOneOnOneGameMode()
{
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
		SpawnedPawn = GetWorld()->SpawnActor<APawn>(SeekerClass, SpawnLocation, SpawnRotation);
	}
	else if (PlayerCount == 2 && HiderClass != nullptr)
	{
		SpawnedPawn = GetWorld()->SpawnActor<APawn>(HiderClass, SpawnLocation, SpawnRotation);
	}

	if (SpawnedPawn && NewPlayer)
	{
		NewPlayer->Possess(SpawnedPawn);
	}
}