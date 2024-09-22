// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "CBall.generated.h"

UCLASS()
class UNREALTIAOQI_API ACBall : public AActor
{
	GENERATED_BODY()
	
public:	
	UMaterialInterface* Material;
	ACBall();
	virtual void initColor(const FLinearColor& TargetColor) {};
	virtual void setType(const int typ) {};

/*
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
*/
};
