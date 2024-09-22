// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NumberShow.generated.h"

UCLASS()
class UNREALTIAOQI_API ANumberShow : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANumberShow();
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* num;
	UPROPERTY(EditAnywhere)
	int n = 0;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
