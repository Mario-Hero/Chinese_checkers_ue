// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Camera/CameraComponent.h>
#include "TiaoQi.generated.h"
// -----x
// |
// |
// |
// y
class ACBall;
class AplayerAdder;
class UTiaoQiWidget;
class ANumberShow;
class BallAnimation {
public:
	constexpr static float H = 100;
	constexpr static float ANIMATION_TIME = 1.4f;   // 10 DEVELOP 1.5NORMAL
	bool enable = false;
	int index = 0;
	float time = 0;
	bool goNextStep = false;
	std::vector<FVector3d> frame;
	int playerIndex = 0;
	ACBall* ballPtr = nullptr;
	BallAnimation() {};
	void clear();
};


UCLASS()
class UNREALTIAOQI_API ATiaoQi : public APawn
{
	GENERATED_BODY()

public:
	UStaticMeshComponent* selectCircle;
	UStaticMeshComponent* playerTriangle;
	UStaticMeshComponent* boardMesh;
	UMaterialInstanceDynamic* triangleMaterial;
	UMaterialInterface* triangleStaticMaterial;
	UCameraComponent* OurCamera;
	UTiaoQiWidget* widget;
	ACBall* middleBall;
	TArray<ANumberShow*> nums;
	const FString SlotNameString = "SaveSlot";
	const int UserIndex = 0;
	UPROPERTY(BlueprintReadWrite)
	FPostProcessSettings pps;
	bool gameStarted = false;
	USoundWave* jumpSoundWave[6];
	USoundWave* buttonSoundWave[6];
	UStaticMesh* numMesh[6];
	ATiaoQi();

private:
	BallAnimation animation;
	int tiaoqiMode = 0; //board type
	int ballMode = 0; //ball type
	std::vector<int> seq;
	std::vector<int> finSeq;
	std::vector<int> finSeqTemp;
	AplayerAdder* playerAdderArray[6];
	UStaticMesh* board1Mesh = nullptr;
	UStaticMesh* board2Mesh = nullptr;
	//std::array<AplayerAdder*, 6> ;
	void drawTrace(const int i);
	void showLastStep();
	void loadGameSave();
	void saveGame();
	void addTime(const float DeltaTime);
	void ErrorSound();
	void checkThisPlayer();
	void flushWinner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay();
	FVector getMouseWorldLocation();
	void MouseClick();
	void CameraHeight(float Value);
	UFUNCTION(BlueprintCallable)
	void NextStep();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void selectPlayer();
	void ChangePlayerSymbol();
	void notifySelectPlayer(AplayerAdder* originSignal);
    UFUNCTION(BlueprintCallable)
    int gameStart();
	UFUNCTION(BlueprintCallable)
	void setWidget(UTiaoQiWidget* w=nullptr);
	UFUNCTION(BlueprintCallable)
	void setSkin(int typ);
	UFUNCTION(BlueprintCallable)
	void setBallSkin(int typ);
	UFUNCTION(BlueprintCallable)
	void updatePPS();
	UFUNCTION(BlueprintCallable)
	void setNums(TArray<ANumberShow*> arr);
};
