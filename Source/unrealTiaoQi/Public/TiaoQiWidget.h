// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TiaoQiWidget.generated.h"

/**
 * 
 */
UCLASS()
class UNREALTIAOQI_API UTiaoQiWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	bool bIsFocusable = true;
	UFUNCTION(BlueprintNativeEvent)
	void humanFinish();
	virtual void humanFinish_Implementation() {};
	UFUNCTION(BlueprintNativeEvent)
	void allFinish();
	virtual void allFinish_Implementation() {};
	UFUNCTION(BlueprintNativeEvent)
	void setSkinType(int typ);
	virtual void setSkinType_Implementation(int typ) {};
};
