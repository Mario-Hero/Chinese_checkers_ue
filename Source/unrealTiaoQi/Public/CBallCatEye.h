// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CBall.h"
#include "CBallCatEye.generated.h"

/**
 * 
 */
UCLASS()
class UNREALTIAOQI_API ACBallCatEye : public ACBall
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UStaticMeshComponent* Sphere;
	UStaticMeshComponent* CatEye;
	ACBallCatEye();
	virtual void initColor(const FLinearColor& TargetColor) override;
	
};
