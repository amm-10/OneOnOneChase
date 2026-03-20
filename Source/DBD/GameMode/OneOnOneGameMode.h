#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OneOnOneGameMode.generated.h"

UCLASS()
class DBD_API AOneOnOneGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// UHT 에러 방지용 기본 생성자 추가
	AOneOnOneGameMode();

	// 유저 접속 시 실행되는 함수
	virtual void PostLogin(APlayerController* NewPlayer) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<APawn> SeekerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<APawn> HiderClass;
};