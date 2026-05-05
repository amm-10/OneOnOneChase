// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ledge.generated.h"

UCLASS()
class DBD_API ALedge : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALedge();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* LedgeMesh;

	UPROPERTY()
	TArray<class UBoxComponent*> InteractionBoxes;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Location")
	FVector ArriveLocationOffset;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void BP_OnHiderClimb(AActor* Hider, FVector StartLoc, FVector EndLoc);


public:
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void DoInteract(AActor* Interactor);


	// 블루프린트에서 호출할 수 있도록 UFUNCTION 추가
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	FTransform GetSnapTransform(AActor* Interactor);
};
