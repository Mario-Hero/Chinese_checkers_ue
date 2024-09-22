// Fill out your copyright notice in the Description page of Project Settings.


#include "CBallPure.h"
const double BALL_SCALE = 0.35;

void ACBallPure::initColor(const FLinearColor& TargetColor)
{
	col = TargetColor;
	switch (type) {
	case 1: {
		Material = LoadObject<UMaterial>(nullptr, TEXT("/Script/Engine.Material'/Game/Material/Ball1.Ball1'"));
		break;
	}
	case 2: {
		Material = LoadObject<UMaterial>(nullptr, TEXT("/Script/Engine.Material'/Game/Material/Ball2.Ball2'"));
		break;
	}
	case 3: {
		Material = LoadObject<UMaterial>(nullptr, TEXT("/Script/Engine.Material'/Game/Material/Ball3.Ball3'"));
		break;
	}
	case 4: {
		Material = LoadObject<UMaterial>(nullptr, TEXT("/Script/Engine.Material'/Game/Material/Ball4.Ball4'"));
		break;
	}
	case 5: {
		Material = LoadObject<UMaterial>(nullptr, TEXT("/Script/Engine.Material'/Game/Material/Ball5.Ball5'"));
		break;
	}
	default: {
		Material = LoadObject<UMaterial>(nullptr, TEXT("/Script/Engine.Material'/Game/Material/Ball1.Ball1'"));
		break;
	}
	}
	UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Material, this);
	DynMaterial->SetVectorParameterValue(FName(TEXT("Color")), col);
	Sphere->SetMaterial(0, DynMaterial);
}

void ACBallPure::setType(const int typ)
{
	type = typ;
}

ACBallPure::ACBallPure()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	Sphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));

	Sphere->SetStaticMesh(SphereAsset.Object);
	Sphere->SetWorldScale3D(FVector{ BALL_SCALE,BALL_SCALE,BALL_SCALE });
	Sphere->SetupAttachment(RootComponent);
	SetActorRotation(FRotator3d(FMath::FRandRange(-180.0, 180.0), FMath::FRandRange(-180.0, 180.0), FMath::FRandRange(-180.0, 180.0)));
}

