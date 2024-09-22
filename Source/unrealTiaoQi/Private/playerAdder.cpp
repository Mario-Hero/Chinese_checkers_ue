// Fill out your copyright notice in the Description page of Project Settings.

#include "playerAdder.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWave.h"
#include "TiaoQi.h"

const double PLUS_SCALE = 0.2;
const double CPU_SCALE = 0.15;
const double PLAYER_SCALE = 0.25;
const double MESH_DISTANCE = 50;

const FVector3d AplayerAdder::TARGET_SELECTCOMPONENT_SCALE = FVector3d(1, 1, 1);
const FVector3d AplayerAdder::TARGET_MIN_SCALE = FVector3d(0.001, 0.001, 0.001);
const FVector3d AplayerAdder::TARGET_PLUS_SCALE = FVector3d(PLUS_SCALE, PLUS_SCALE, PLUS_SCALE);
const FVector3d AplayerAdder::ORIGINAL_CPU_SCALE = FVector3d(CPU_SCALE, CPU_SCALE, CPU_SCALE);
const FVector3d AplayerAdder::ORIGINAL_HUMAN_SCALE = FVector3d(PLAYER_SCALE, PLAYER_SCALE, PLAYER_SCALE);
const FVector3d AplayerAdder::SELECT_CPU_SCALE = ORIGINAL_CPU_SCALE * 1.5;
const FVector3d AplayerAdder::SELECT_HUMAN_SCALE = ORIGINAL_HUMAN_SCALE * 1.5;
const FVector3d AplayerAdder::ORIGINAL_CPU_LOC = FVector3d(0, MESH_DISTANCE, 0);
const FVector3d AplayerAdder::ORIGINAL_HUMAN_LOC = FVector3d(0, -MESH_DISTANCE, 0);

// Sets default values
AplayerAdder::AplayerAdder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	plus = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plus"));
	meshCPU = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("meshCPU"));
	meshPlayer = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("meshPlayer"));
	selectComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("selectComponent"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> plusAsset(TEXT("/Script/Engine.StaticMesh'/Game/Mesh/plus.plus'"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> cpuAsset(TEXT("/Script/Engine.StaticMesh'/Game/Mesh/cpu.cpu'"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> playerAsset(TEXT("/Script/Engine.StaticMesh'/Game/Mesh/human.human'"));

	static ConstructorHelpers::FObjectFinder<UMaterial> SimpleMaterial(TEXT("/Script/Engine.Material'/Game/Material/cpuMaterial.cpuMaterial'"));
	Material = SimpleMaterial.Object;

	plus->SetStaticMesh(plusAsset.Object);
	meshCPU->SetStaticMesh(cpuAsset.Object);
	meshPlayer->SetStaticMesh(playerAsset.Object);
	plus->OnReleased.AddDynamic(this, &AplayerAdder::plusClicked);   //������¼�
	plus->OnInputTouchEnd.AddDynamic(this, &AplayerAdder::plusTouched);   //����������¼�
	plus->SetRelativeScale3D(TARGET_PLUS_SCALE);
	meshCPU->SetRelativeScale3D(ORIGINAL_CPU_SCALE);
	meshPlayer->SetRelativeScale3D(ORIGINAL_HUMAN_SCALE);
	plus->SetRelativeRotation(FRotator3d(90, 0, 0));
	plus->SetupAttachment(RootComponent);
	selectComponent->SetupAttachment(RootComponent);
	meshCPU->SetupAttachment(selectComponent);
	meshPlayer->SetupAttachment(selectComponent);
	meshCPU->SetRelativeLocation(ORIGINAL_CPU_LOC);
	meshPlayer->SetRelativeLocation(ORIGINAL_HUMAN_LOC);
	selectComponent->SetRelativeLocation(FVector3d(0, 0, 10));
	selectComponent->SetWorldScale3D(FVector3d(0.001, 0.001, 0.001));
	selectComponent->SetWorldRotation(FRotator(90,-90,0));
	selectComponent->SetVisibility(false);
	meshCPU->OnReleased.AddDynamic(this, &AplayerAdder::meshCPUClicked);   //������¼�
	meshCPU->OnInputTouchEnd.AddDynamic(this, &AplayerAdder::meshCPUTouched);   //����������¼�
	meshPlayer->OnReleased.AddDynamic(this, &AplayerAdder::meshPlayerClicked);   //������¼�
	meshPlayer->OnInputTouchEnd.AddDynamic(this, &AplayerAdder::meshPlayerTouched);   //����������¼�
}

// Called when the game starts or when spawned
void AplayerAdder::BeginPlay()
{
	Super::BeginPlay();
	
}

FVector3d AplayerAdder::lerpVector(const FVector3d& a, const FVector3d& b, const float DeltaTime)
{
	return (b - a)* DeltaTime*20 + a;
}


// Called every frame
void AplayerAdder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (state == 0) {
		plus->AddRelativeRotation(FRotator3d(0, DeltaTime * 30, 0));
	}
	if (animating) {
		if (state == 1) {
			selectComponent->SetRelativeScale3D(lerpVector(selectComponent->GetRelativeScale3D(), TARGET_SELECTCOMPONENT_SCALE, DeltaTime));
			plus->SetRelativeScale3D(lerpVector(plus->GetRelativeScale3D(), TARGET_MIN_SCALE, DeltaTime));
			meshCPU->SetRelativeScale3D(lerpVector(meshCPU->GetRelativeScale3D(), ORIGINAL_CPU_SCALE, DeltaTime));
			meshPlayer->SetRelativeScale3D(lerpVector(meshPlayer->GetRelativeScale3D(), ORIGINAL_HUMAN_SCALE, DeltaTime));
			meshCPU->SetRelativeLocation(lerpVector(meshCPU->GetRelativeLocation(), ORIGINAL_CPU_LOC, DeltaTime));
			meshPlayer->SetRelativeLocation(lerpVector(meshPlayer->GetRelativeLocation(), ORIGINAL_HUMAN_LOC, DeltaTime));

			if (selectComponent->GetRelativeScale3D().Equals(TARGET_SELECTCOMPONENT_SCALE, 0.001) 
				&& plus->GetRelativeScale3D().Equals(TARGET_MIN_SCALE, 0.001)
				&& meshCPU->GetRelativeScale3D().Equals(ORIGINAL_CPU_SCALE, 0.001)
				&& meshPlayer->GetRelativeScale3D().Equals(ORIGINAL_HUMAN_SCALE, 0.001)
				&& meshCPU->GetRelativeLocation().Equals(ORIGINAL_CPU_LOC, 0.001)
				&& meshPlayer->GetRelativeLocation().Equals(ORIGINAL_HUMAN_LOC, 0.001)) {
				animating = false;
				plus->SetVisibility(false);
			}
		}
		else if (state == 0) {
			selectComponent->SetRelativeScale3D(lerpVector(selectComponent->GetRelativeScale3D(), TARGET_MIN_SCALE, DeltaTime));
			plus->SetRelativeScale3D(lerpVector(plus->GetRelativeScale3D(), TARGET_PLUS_SCALE, DeltaTime));
			if (selectComponent->GetRelativeScale3D().Equals(TARGET_MIN_SCALE, 0.001) && plus->GetRelativeScale3D().Equals(TARGET_PLUS_SCALE, 0.001)) {
				animating = false;
				selectComponent->SetVisibility(false);
			}
		}
		else if (state == 2) { //CPU
			selectComponent->SetRelativeScale3D(lerpVector(selectComponent->GetRelativeScale3D(), TARGET_SELECTCOMPONENT_SCALE, DeltaTime));
			plus->SetRelativeScale3D(lerpVector(plus->GetRelativeScale3D(), TARGET_MIN_SCALE, DeltaTime));
			meshPlayer->SetRelativeScale3D(lerpVector(meshPlayer->GetRelativeScale3D(), TARGET_MIN_SCALE, DeltaTime));
			meshCPU->SetRelativeScale3D(lerpVector(meshCPU->GetRelativeScale3D(), SELECT_CPU_SCALE, DeltaTime));
			meshCPU->SetRelativeLocation(lerpVector(meshCPU->GetRelativeLocation(), FVector3d::ZeroVector, DeltaTime));
			if (selectComponent->GetRelativeScale3D().Equals(TARGET_SELECTCOMPONENT_SCALE, 0.001) 
				&& plus->GetRelativeScale3D().Equals(TARGET_MIN_SCALE, 0.001)
				&& meshPlayer->GetRelativeScale3D().Equals(TARGET_MIN_SCALE, 0.001)
				&& meshCPU->GetRelativeScale3D().Equals(SELECT_CPU_SCALE, 0.001)
				&& meshCPU->GetRelativeLocation().Equals(FVector3d::ZeroVector, 0.001)) {
				animating = false;
				plus->SetVisibility(false);
				meshPlayer->SetVisibility(false);
			}
		}
		else if (state == 3) { //HUMAN
			selectComponent->SetRelativeScale3D(lerpVector(selectComponent->GetRelativeScale3D(), TARGET_SELECTCOMPONENT_SCALE, DeltaTime));
			plus->SetRelativeScale3D(lerpVector(plus->GetRelativeScale3D(), TARGET_MIN_SCALE, DeltaTime));
			meshCPU->SetRelativeScale3D(lerpVector(meshCPU->GetRelativeScale3D(), TARGET_MIN_SCALE, DeltaTime));
			meshPlayer->SetRelativeScale3D(lerpVector(meshPlayer->GetRelativeScale3D(), SELECT_HUMAN_SCALE, DeltaTime));
			meshPlayer->SetRelativeLocation(lerpVector(meshPlayer->GetRelativeLocation(), FVector3d::ZeroVector, DeltaTime));
			if (selectComponent->GetRelativeScale3D().Equals(TARGET_SELECTCOMPONENT_SCALE, 0.001)
				&& plus->GetRelativeScale3D().Equals(TARGET_MIN_SCALE, 0.001)
				&& meshCPU->GetRelativeScale3D().Equals(TARGET_MIN_SCALE, 0.001)
				&& meshPlayer->GetRelativeScale3D().Equals(SELECT_HUMAN_SCALE, 0.001)
				&& meshPlayer->GetRelativeLocation().Equals(FVector3d::ZeroVector, 0.001)) {
				animating = false;
				plus->SetVisibility(false);
				meshCPU->SetVisibility(false);
			}
		}
		
	}
}

void AplayerAdder::setColor(const FLinearColor& col)
{
	UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Material, this);
	DynMaterial->SetVectorParameterValue(FName(TEXT("Color")), col);
	plus->SetMaterial(0, DynMaterial);
	meshCPU->SetMaterial(2, DynMaterial);
	meshPlayer->SetMaterial(0, DynMaterial);
}

void AplayerAdder::setParent(ATiaoQi* p)
{
	parent = p;
}

void AplayerAdder::setIndex(int i)
{
	index = i;
}

void AplayerAdder::clickNotifyParent()
{
	parent->notifySelectPlayer(this);
}


bool AplayerAdder::noClick(AplayerAdder* originSignal)
{
	if (originSignal != this) {
		if (state == 1) {
			state = 0;
			animating = true;
			plus->SetVisibility(true);
			return true;
		}
	}
	return false;
}

void AplayerAdder::remove()
{
	plus = nullptr;
	meshCPU = nullptr;
	meshPlayer = nullptr;
	selectComponent = nullptr;
	this->Destroy();
}


void AplayerAdder::plusClicked(UPrimitiveComponent* ClickedComp, FKey key=FKey())
{
	if (animating) return;
	state = 1;
	animating = true;
	selectComponent->SetVisibility(true);
	playSound();
	clickNotifyParent();
}

void AplayerAdder::playSound() {
	UGameplayStatics::PlaySoundAtLocation(this, parent->buttonSoundWave[FMath::RandRange(0, 4)], this->K2_GetActorLocation());
}


void AplayerAdder::plusTouched(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent)
{
	plusClicked(TouchedComponent);
}

void AplayerAdder::meshCPUClicked(UPrimitiveComponent* ClickedComp, FKey key = FKey())
{
	if (animating) return;
	if (selectOne == SELECT::CPU) {
		state = 1;
		selectOne = SELECT::NONE;
		meshPlayer->SetVisibility(true);
	}
	else {
		state = 2; //CPU
		selectOne = SELECT::CPU;
	}
	meshPlayer->SetVisibility(true);
	meshCPU->SetVisibility(true);
	animating = true;
	playSound();
}


void AplayerAdder::meshCPUTouched(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent)
{
	meshCPUClicked(TouchedComponent);
}

void AplayerAdder::meshPlayerClicked(UPrimitiveComponent* ClickedComp, FKey key = FKey())
{
	if (animating) return;
	if (selectOne == SELECT::HUMAN) {
		state = 1;
		selectOne = SELECT::NONE;
	}
	else {
		state = 3; //HUMAN
		selectOne = SELECT::HUMAN;
	}
	meshPlayer->SetVisibility(true);
	meshCPU->SetVisibility(true);
	animating = true;
	playSound();
}


void AplayerAdder::meshPlayerTouched(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent)
{
	meshPlayerClicked(TouchedComponent);
}




