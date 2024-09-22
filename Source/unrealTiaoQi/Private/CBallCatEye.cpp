// Fill out your copyright notice in the Description page of Project Settings.


#include "CBallCatEye.h"
const double BALL_SCALE = 0.35;
const double CATEYE_SCALE = 0.15;

ACBallCatEye::ACBallCatEye()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	Sphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere"));
	CatEye = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CatEye"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CatEyeAsset(TEXT("/Script/Engine.StaticMesh'/Game/Mesh/CatEye.CatEye'"));
	static ConstructorHelpers::FObjectFinder<UMaterial> sphereMaterial(TEXT("/Script/Engine.Material'/Game/Material/Glass.Glass'"));
	static ConstructorHelpers::FObjectFinder<UMaterial> CatEyeMaterial(TEXT("/Script/Engine.Material'/Game/Material/CatEye.CatEye'"));

	Sphere->SetStaticMesh(SphereAsset.Object);
	CatEye->SetStaticMesh(CatEyeAsset.Object);
	Material = CatEyeMaterial.Object;
	Sphere->SetWorldScale3D(FVector{ BALL_SCALE,BALL_SCALE,BALL_SCALE });
	CatEye->SetWorldScale3D(FVector{ CATEYE_SCALE,CATEYE_SCALE,CATEYE_SCALE });
	Sphere->SetupAttachment(RootComponent);
	CatEye->SetupAttachment(RootComponent);
	Sphere->SetMaterial(0, sphereMaterial.Object);
	SetActorRotation(FRotator3d(FMath::FRandRange(-180.0, 180.0), FMath::FRandRange(-180.0, 180.0), FMath::FRandRange(-180.0, 180.0)));
}

void ACBallCatEye::initColor(const FLinearColor& TargetColor)
{
	UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Material, this);
	DynMaterial->SetVectorParameterValue(FName(TEXT("Color")), TargetColor);
	CatEye->SetMaterial(0, DynMaterial);
}
