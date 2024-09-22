// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "playerAdder.generated.h"

class ATiaoQi;

UCLASS()
class UNREALTIAOQI_API AplayerAdder : public AActor
{
	GENERATED_BODY()


private:
	bool animating = false;
	int state = 0;//0�Ӻ� 1CPU���� 2CPU 3��
	static const FVector3d TARGET_SELECTCOMPONENT_SCALE;
	static const FVector3d TARGET_MIN_SCALE;
	static const FVector3d TARGET_PLUS_SCALE;
	static const FVector3d SELECT_CPU_SCALE;
	static const FVector3d SELECT_HUMAN_SCALE;
	static const FVector3d ORIGINAL_CPU_LOC;
	static const FVector3d ORIGINAL_HUMAN_LOC;
	static const FVector3d ORIGINAL_CPU_SCALE;
	static const FVector3d ORIGINAL_HUMAN_SCALE;
	UMaterialInterface* Material;
	void playSound();

public:	
	// Sets default values for this actor's properties
	AplayerAdder();
	UFUNCTION()
	void plusClicked(UPrimitiveComponent* ClickedComp, FKey key);
	UFUNCTION()
	void plusTouched(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent);
	UFUNCTION()
	void meshCPUClicked(UPrimitiveComponent* ClickedComp, FKey key);
	UFUNCTION()
	void meshCPUTouched(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent);
	UFUNCTION()
	void meshPlayerClicked(UPrimitiveComponent* ClickedComp, FKey key);
	UFUNCTION()
	void meshPlayerTouched(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent);
	enum SELECT {NONE=0, CPU=1, HUMAN=2};
	int selectOne = SELECT::NONE;
	int index = -1;
	UStaticMeshComponent* plus;
	UStaticMeshComponent* meshCPU;
	UStaticMeshComponent* meshPlayer;
	UStaticMeshComponent* selectComponent;
	ATiaoQi* parent;
	static FVector3d lerpVector(const FVector3d& a, const FVector3d& b, const float DeltaTime);
	virtual void Tick(float DeltaTime) override;
	void setColor(const FLinearColor& col);
	void setParent(ATiaoQi* p);
	void setIndex(int i);
	void clickNotifyParent();
	bool noClick(AplayerAdder* originSignal);  // true 被取消 false 没有改动
	void remove();
	


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};

