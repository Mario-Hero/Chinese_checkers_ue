// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MySaveGame.generated.h"

/**
 * 
 */
UCLASS()
class UNREALTIAOQI_API UMySaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UMySaveGame();
	UPROPERTY(VisibleAnywhere, Category = Basic)
		FString SaveSlotName;
	UPROPERTY(VisibleAnywhere, Category = Basic)
		uint32 UserIndex;
	UPROPERTY(BlueprintReadWrite)
	int tiaoqiMode = 0; //board mode
	UPROPERTY(BlueprintReadWrite)
	int ballMode = 0;
	UPROPERTY(BlueprintReadWrite)
	int lang = 1; //-1 NOT_SET 0 CN 1 EN
};
