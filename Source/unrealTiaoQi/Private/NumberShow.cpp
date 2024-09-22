// Fill out your copyright notice in the Description page of Project Settings.


#include "NumberShow.h"

// Sets default values
ANumberShow::ANumberShow()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	num = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	num->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ANumberShow::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANumberShow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

