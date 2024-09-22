// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CBall.h"
#include "CBallPure.generated.h"

/**
 * 
 */
UCLASS()
class UNREALTIAOQI_API ACBallPure : public ACBall
{
	GENERATED_BODY()
	
public:
	int type = 2;
	FLinearColor col;
	UStaticMeshComponent* Sphere;
	UMaterial* Material;
	virtual void initColor(const FLinearColor& TargetColor) override;
	virtual void setType(const int typ) override;
	ACBallPure();
};
