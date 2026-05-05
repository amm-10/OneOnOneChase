// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Doghole.generated.h"

UCLASS()
class DBD_API ADoghole : public AActor
{
	GENERATED_BODY()
	
public:
	ADoghole();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* DogholeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* FrontInteractionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* BackInteractionBox;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void BP_OnHiderVault(AActor* Hider, FVector StartLoc, FVector EndLoc);

public:
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void DoInteract(AActor* Interactor);


	// 블루프린트에서 호출할 수 있도록 UFUNCTION 추가
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	FTransform GetSnapTransform(AActor* Interactor);
};
