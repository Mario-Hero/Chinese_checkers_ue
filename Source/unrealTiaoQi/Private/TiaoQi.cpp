// Fill out your copyright notice in the Description page of Project Settings.

#include "TiaoQi.h"
#include "Math/UnrealMathUtility.h"
#include "TiaoQiWidget.h"
#include "CBall.h"
#include "CBallCatEye.h"
#include "CBallPure.h"
#include "ColorConst.h"
#include "playerAdder.h"
#include "MySaveGame.h"
#include "NumberShow.h"
#include <Kismet/GameplayStatics.h>
#include <Camera/CameraComponent.h>
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Sound/SoundWave.h"
#include <utility>
#include <math.h>
#include <vector>
#include <map>
#include <iostream>
#include <queue>
#include <array>
#include <tuple>
#include <algorithm>

// -----x
// |
// |
// |
// y

using pos = std::pair<int, int>;
using vec2 = std::pair<double, double>;
UWorld* mainWorld=nullptr;

const FString jumpSound[5] = { TEXT("/Script/Engine.SoundWave'/Game/soundFX/Jump/s1.s1'"), TEXT("/Script/Engine.SoundWave'/Game/soundFX/Jump/s2.s2'"), TEXT("/Script/Engine.SoundWave'/Game/soundFX/Jump/s3.s3'"),
						 TEXT("/Script/Engine.SoundWave'/Game/soundFX/Jump/s4.s4'"),TEXT("/Script/Engine.SoundWave'/Game/soundFX/Jump/s5.s5'") };
const FString buttonSound[5] = { TEXT("/Script/Engine.SoundWave'/Game/soundFX/Button/UI_023.UI_023'"), TEXT("/Script/Engine.SoundWave'/Game/soundFX/Button/UI_024.UI_024'"), TEXT("/Script/Engine.SoundWave'/Game/soundFX/Button/UI_025.UI_025'"),
						 TEXT("/Script/Engine.SoundWave'/Game/soundFX/Button/UI_026.UI_026'"),TEXT("/Script/Engine.SoundWave'/Game/soundFX/Button/UI_027.UI_027'") };

const FString nextStepSound[5] = {
TEXT("/Script/Engine.SoundWave'/Game/soundFX/humanNextStep/button01_mp3.button01_mp3'"),
TEXT("/Script/Engine.SoundWave'/Game/soundFX/humanNextStep/button03_mp3.button03_mp3'"),
TEXT("/Script/Engine.SoundWave'/Game/soundFX/humanNextStep/button04_mp3.button04_mp3'"),
TEXT("/Script/Engine.SoundWave'/Game/soundFX/humanNextStep/button05_mp3.button05_mp3'"),
TEXT("/Script/Engine.SoundWave'/Game/soundFX/humanNextStep/button06_mp3.button06_mp3'") };

const FString firstSound = TEXT("/Script/Engine.SoundWave'/Game/soundFX/First/Win_sound.Win_sound'");

const FString numberMeshString[6] = {TEXT("/Script/Engine.StaticMesh'/Game/Mesh/Numbers/1.1'"),TEXT("/Script/Engine.StaticMesh'/Game/Mesh/Numbers/2.2'"),
TEXT("/Script/Engine.StaticMesh'/Game/Mesh/Numbers/3.3'") ,TEXT("/Script/Engine.StaticMesh'/Game/Mesh/Numbers/4.4'") ,TEXT("/Script/Engine.StaticMesh'/Game/Mesh/Numbers/5.5'"),
TEXT("/Script/Engine.StaticMesh'/Game/Mesh/Numbers/6.6'") };

void BallAnimation::clear() {
	frame.clear();
	index = 0;
	enable = false;
	ballPtr = nullptr;
	playerIndex = 0;
	time = 0;
	enable = false;
}
void ATiaoQi::addTime(const float DeltaTime) {
	animation.time += DeltaTime * BallAnimation::ANIMATION_TIME;
	bool nextIndex = false;
	if (animation.time > 1) {
		nextIndex = true;
		animation.time = 1;
	}
	const FVector3d V = animation.frame[animation.index - 1] - animation.frame[animation.index];
	const float L = V.Size();
	const float x = animation.time * L;
	const float y = 4 * BallAnimation::H * x * (L - x) / (L * L);
	const FVector3d T = animation.frame[animation.index] + V * animation.time + FVector3d(0, 0, y);
	if (animation.ballPtr) {
		animation.ballPtr->SetActorLocation(T, false);
	}
	if (nextIndex) {
		--animation.index;
		animation.time = 0;
		if (animation.index == 0) {
			animation.enable = false;
		}
		UGameplayStatics::PlaySoundAtLocation(this, jumpSoundWave[FMath::RandRange(0, 4)], T);
	}
}

struct step {
	pos start;
	pos end;
	bool operator == (const step& b) const {
		return start == b.start && end == b.end;
	}
	const step operator - () const {
		return step{ end, start };
	}
};
using thinkTreeRes = std::tuple<int, int, int>;  //��һ��������Ҫ���غϻ�ʤ�����û��ʤ��Ϊ0��һ����ʤΪ1

enum RES_NUMBER { RES_DEPTH = 0, RES_PATH = 1, RES_SCORE = 2};

/*
const int RES_DEPTH = 0;
const int RES_PATH = 1;
const int RES_SCORE = 2;
*/
const int NO_DEPTH = 999999;
using longStep = step;
using singleStep = step;
using stepVector = std::vector<step>;

const double radius = 19.141;
const double gapX = 21.447;

const float SCORE_FACTOR = 0.6;
const int TREE_DEPTH = 6;
const int DEPTH_FACTOR_SELF = 2;
const int DEPTH_FACTOR_OTHER = 1;

enum FINISH_LINE { EMPTY = 6, UNDEFINED = 7, FIN_N = 0, FIN_S = 1, FIN_WN = 2, FIN_EN = 3, FIN_WS = 4, FIN_ES = 5 };
enum STEP { INVALID_MOVE=0, ONE_MOVE=1, JUMP_MOVE=2};
using PLAYER_ID = FINISH_LINE;

const int FIN_S_INITIAL_POS[10][2] = { 0,12,1,11,1,13,2,10,2,12,2,14,3,9,3,11,3,13,3,15 };  //正确的
const double ROTATION_ID[6] = { 0, 180, 300, 60, 240, 120}; //回归正位所需的逆时针旋转角度
const int FIN_ES_INITIAL_POS[10][2] = { 4,0,4,2,4,4,4,6,5,1,5,3,5,5,6,2,6,4,7,3 };
const int SINGLE_MOVE[6][2] = { 0, 2, 0, -2, 1, 1, 1, -1, -1, 1, -1, -1 };
const pos PLAYER_ADDER_INITIAL_POS[6] = { {14,12},{2,12},{11,21},{11,3},{5,21},{5,3} };
const pos NUM_INITIAL_POS[6] = { {10,12},{6,12},{9,15},{9,9},{7,15},{7,9} };
const pos FIN_SYMBOL_POS[6] = { {2,12},{14,12},{5,3},{5,21},{11,3},{11,21} };

const pos FIN_POINT[6][2] = { {{3,9},{3,15}}, {{13,15},{13,9}}, {{7,3},{4,6}}, {{4,18},{7,21} }, {{12,6},{9,3}} , {{9,21}, {12,18}} };

enum CPU_TYPE { CPU_PLAYER = true, HUMAN_PLAYER = false };
PLAYER_ID checker[17][25] = { EMPTY };
const step EMPTY_STEP = { std::make_pair(0,0), std::make_pair(0,0) };

class thinkTree {
public:
	std::vector<thinkTree*> move;
	int score = 0;
	PLAYER_ID id;
	int pawni = 0;
	longStep step;
	int parent = -1; // JUMP PARENT INDEX
	thinkTree() {
		id = EMPTY;
	}
	thinkTree(const PLAYER_ID playerId, const int sc, const longStep st, const int i) { //��Ҫ�洢id����Ϊ���Ѿ�����˵�
		score = sc;
		step = st;
		id = playerId;
		pawni = i;
	}
	~thinkTree() {
		for (thinkTree* t : move) delete t;
	}
	void clear() {
		for (thinkTree* t : move) delete t;
		score = 0;
		pawni = 0;
		parent = -1;
	}
};
using pathScore = std::pair<int, int>;

struct player {
	PLAYER_ID id = EMPTY;
	CPU_TYPE cpu = CPU_PLAYER;
	int pawnFinished = 0;
	int nowSelectPawn = -1;
	STEP lastStepType = INVALID_MOVE;
	FLinearColor color;
	pos oriPos = std::make_pair(0,0);
	std::array<pos, 10> pawnList;
	std::array<ACBall*, 10> ballList;
};

thinkTree* root = new thinkTree();
std::vector<pos> dfsVector;
longStep lastStep = EMPTY_STEP;

//stepVector lastStepVector;

class playerSystem {
public:
	std::vector<player> m;
	playerSystem() {};
	~playerSystem() {
		m.clear();
	}
	int nowIndex = 0;
	bool hasCPUPlayer = false;
	bool nextPlayer() {  // true:进入下一轮 false:没有进入下一轮
		bool returnValue = false;
		for (int i = 0; i < m.size(); ++i) {
			nowIndex = (nowIndex + 1) % m.size();
			if (nowIndex == 0) returnValue = true;
			if (m[nowIndex].pawnFinished < 10) return returnValue;
		}
		return returnValue;
	}
	int getNextPlayer(const int nowP) {
		int b = nowP;
		for (int i = 0; i < m.size(); ++i) {
			b = (b + 1) % m.size();
			if (m[b].pawnFinished < 10) return b;
		}
		return b;
	}
	player& getNowPlayer() {
		return m[nowIndex];
	}
	bool allFinished() {
		for (size_t i = 0; i < m.size(); ++i) {
			if (m[i].pawnFinished < 10) return false;
		}
		return true;
	}
	void addPlayer(PLAYER_ID id, bool is_CPU) {
		const CPU_TYPE t = is_CPU ? CPU_TYPE::CPU_PLAYER : CPU_TYPE::HUMAN_PLAYER;
		player p = {id,t,0};
		this->m.emplace_back(p);
	}
	void clear() {
		m.clear();
		nowIndex = 0;
		hasCPUPlayer = false;
	}
};
playerSystem allPlayer = playerSystem();
longStep stepVectorToLongStep(const stepVector& v) {
	longStep t;
	t.start = v.front().start;
	t.end = v.back().end;
	return t;
}

void coutVector(const FVector3d& vec) {
	UE_LOG
	(
		LogTemp,
		Display,
		TEXT("x:%f, y:%f, z:%f"),
		vec[0], vec[1], vec[2]
	);
}
void coutPos(const pos& ps) {
	UE_LOG
	(
		LogTemp,
		Display,
		TEXT("i:%d, j:%d"),
		ps.first, ps.second
	);
}
void coutVec2(const vec2& ps) {
	UE_LOG
	(
		LogTemp,
		Display,
		TEXT("x:%f, y:%f"),
		ps.first, ps.second
	);
}
void coutStep(const step& st) {
	UE_LOG
	(
		LogTemp,
		Display,
		TEXT("start: i:%d, j:%d"),
		st.start.first, st.start.second
	);
	UE_LOG
	(
		LogTemp,
		Display,
		TEXT("end: i:%d, j:%d"),
		st.end.first, st.end.second
	);
}
inline int disPoint(const pos& p1, const pos& p2) {
	return (p1.first - p2.first) * (p1.first - p2.first) + (p1.second - p2.second) * (p1.second - p2.second) / 6;
}
int getFinishedPawn(const PLAYER_ID id) {
	for (const player& p : allPlayer.m) {
		if (p.id == id) {
			return p.pawnFinished;
		}
	}
	return 0;
}
bool pointInFinishArea(const pos &p, const FINISH_LINE target) {
	switch (target) {
	case FIN_N: return p.first <= 3;
	case FIN_S: return p.first >= 13;
	case FIN_WN: return p.first + p.second <= 10;
	case FIN_EN: return p.second - p.first >= 14;
	case FIN_WS: return p.first - p.second >= 6;
	case FIN_ES: return p.first + p.second >= 30;
	default: return false;
	}
}


inline int distanceToFinish(const pos& p, const FINISH_LINE target) {
	return pointInFinishArea(p, target) ? disPoint(p, FIN_POINT[target][0]) + disPoint(p, FIN_POINT[target][1]) : 0;
}
inline bool goInFinishArea(const PLAYER_ID id, const step& p) {
	return pointInFinishArea(p.end, id) && !pointInFinishArea(p.start, id);
}
inline bool goInFinishArea(const PLAYER_ID id, const pos& a, const pos& b) {
	return pointInFinishArea(b, id) && !pointInFinishArea(a, id);
}
inline bool goOutFinishArea(const PLAYER_ID id, const step& p) {
	return pointInFinishArea(p.start, id) && !pointInFinishArea(p.end, id);
}
inline bool goOutFinishArea(const PLAYER_ID id, const pos& a, const pos& b) {
	return pointInFinishArea(a, id) && !pointInFinishArea(b, id);
}

vec2 get2DLocation(const int i, const int j) {
	const double x = (j - 12) * gapX;
	const double y = (i * 2 - 16) * radius;
	return { x, y };
}

vec2 get2DLocation(const pos& a) {
	const double x = (a.second - 12) * gapX;
	const double y = (a.first * 2 - 16) * radius;
	return { x, y };
}

vec2 rotatePoint(const vec2& p, const double a) {
	const double r = a * 3.1415926 / 180.0;
	vec2 b;
	b.first = p.second * std::sin(r) + p.first * std::cos(r);
	b.second = p.second * std::cos(r) - p.first * std::sin(r);
	return b;
}

int getScore(const pos& start, const pos& end, const FINISH_LINE target) {
	if (!(pointInFinishArea(start, target) && pointInFinishArea(end, target) )) {
		const vec2 a = rotatePoint(get2DLocation(start), ROTATION_ID[target]);
		const vec2 b = rotatePoint(get2DLocation(end), ROTATION_ID[target]);
		//coutPos(start);
		//coutVec2(a);
		//coutVec2(b);
		return static_cast<int>(a.second - b.second + (std::abs(a.first) - std::abs(b.first))/8);
	}
	else {
		return 0;
	}
}

pos mouseToPos(const float x, const float y) {
	const int i = round(y / static_cast<float>(2 * radius)) + 8;
	const float j = x / static_cast<float>(gapX) + 12.0;
	int jr = round(j);
	if ((jr + i) % 2 != 0) {
		if (j > jr) 
			++jr;
		else 
			--jr;
	}
	return std::make_pair(i, jr);
}

inline bool validPos(const pos& p) {
	return ! (p.first < 0 || p.first > 16 || p.second < 0 || p.second > 24 || checker[p.first][p.second] == UNDEFINED);
}

inline bool hasChess(const int i, const int j) {
	return checker[i][j] != EMPTY && checker[i][j] != UNDEFINED;
}

STEP validSingleStep(const singleStep& s) { //���ж�ԭ��̤��
	//�յ㲻Ϊ��
	if (checker[s.end.first][s.end.second] != EMPTY) return INVALID_MOVE;
	//������������Ϊ��
	if (checker[s.start.first][s.start.second] == UNDEFINED || checker[s.start.first][s.start.second] == EMPTY) return INVALID_MOVE;
	//ƽ��
	if (s.start.first == s.end.first && abs(s.start.second - s.end.second) == 2) return ONE_MOVE;
	if (abs(s.start.first - s.end.first) == 1 && abs(s.start.second - s.end.second) == 1) return ONE_MOVE;
	//����
	if (s.start.second == s.end.second) return INVALID_MOVE;
	const int jb = s.end.second > s.start.second ? 1 : -1;
	if (s.end.first == s.start.first) {
		int mid = s.start.second + s.end.second;
		if (mid % 2 == 1) return INVALID_MOVE;
		mid /= 2;
		if (checker[s.start.first][mid] == EMPTY || checker[s.start.first][mid] == UNDEFINED) return INVALID_MOVE;
		for (int ps = s.start.second + jb * 2; ps != s.end.second; ps += jb * 2) {
			if (checker[s.start.first][ps] != EMPTY && ps != mid) {
				return INVALID_MOVE;
			}
		}
		return JUMP_MOVE;
	}
	else {
		if (abs(s.end.first - s.start.first) != abs(s.end.second - s.start.second)) return INVALID_MOVE;
		const int ib = s.end.first > s.start.first ? 1 : -1;
		int midi = s.start.first + s.end.first;
		if (midi % 2 == 1) return INVALID_MOVE;
		int midj = s.start.second + s.end.second;
		if (midj % 2 == 1) return INVALID_MOVE;
		midi /= 2;
		midj /= 2;
		if (checker[midi][midj] == EMPTY || checker[midi][midj] == UNDEFINED) return INVALID_MOVE;
		int pi = s.start.first + ib;
		int pj = s.start.second + jb;
		for (; pi != s.end.first; pi += ib, pj += jb) {
			if (checker[pi][pj] != EMPTY && pi != midi) {
				return INVALID_MOVE;
			}
		}
		return JUMP_MOVE;
	}
	return INVALID_MOVE;
}

FVector3d getLocation(const int i, const int j) {
	const double x = (j - 12) * gapX;
	const double y = (i * 2 - 16) * radius;
	const double z = 0;
	return FVector3d{ x, y, z };
}

FRotator randomRotator() {
	return FRotator(FMath::RandRange(0.0, 360.0), FMath::RandRange(0.0, 360.0), FMath::RandRange(0.0, 360.0));
}
FVector3d getLocation(const pos& p) {
	const double x = (p.second - 12) * gapX;
	const double y = (p.first * 2 - 16) * radius;
	const double z = 0;
	return FVector3d{ x, y, z };
}

void ATiaoQi::loadGameSave() {
	int tqMode = 0;
	int baMode = 0;
	if (UMySaveGame* LoadedGame = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot("SaveSlot", 0)))
	{
		UE_LOG(LogTemp, Warning, TEXT("Load Game Success."));
		tqMode = LoadedGame->tiaoqiMode;
		baMode = LoadedGame->ballMode;
		//delete LoadedGame;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Load Game Failed."));
		if (UMySaveGame* SaveGameInstance = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass())))
		{
			SaveGameInstance->tiaoqiMode = 0;
			SaveGameInstance->ballMode = 0;

			// 即时保存游戏。
			if (UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotNameString, UserIndex))
			{
				// 成功保存。
			}
		}
	}
	tiaoqiMode = tqMode;
	ballMode = baMode;
}

void initCheckers(int ballMode = 0) {
	lastStep = EMPTY_STEP;
	root->clear();
	root = new thinkTree();
	for (int i = 0; i <= 16; ++i) {
		for (int j = 0; j <= 24; ++j) {
			if ((i + j) % 2 == 1
				|| (i <= 3 && (j + i <= 11 || j - i >= 13))
				|| (i >= 13 && (j + i >= 29 || i - j >= 5))
				|| (i <= 8 && i >= 5 && (i - j >= 5 || i + j >= 29))
				|| (i <= 11 && i >= 8 && (i + j <= 11 || j - i >= 13))) {
				checker[i][j] = UNDEFINED;
			}
			else
				checker[i][j] = EMPTY;
		}
	}
	for (player& p : allPlayer.m) {
		if (p.id < 0 || p.id >=6) continue;
		const FLinearColor col = ColorConst::COLOR_ARRAY[p.id];
		for (int i = 0; i < 10; ++i) {
			int j, k;
			switch (p.id) {
			case FIN_S: {
				j = FIN_S_INITIAL_POS[i][0];
				k = FIN_S_INITIAL_POS[i][1];
				break;
			}
			case FIN_N: {
				j = 16 - FIN_S_INITIAL_POS[i][0];
				k = FIN_S_INITIAL_POS[i][1];
				break;
			}
			case FIN_ES: {
				j = FIN_ES_INITIAL_POS[i][0];
				k = FIN_ES_INITIAL_POS[i][1];
				break;
			}
			case FIN_EN: {
				j = 16 - FIN_ES_INITIAL_POS[i][0];
				k = FIN_ES_INITIAL_POS[i][1];
				break;
			}
			case FIN_WS: {
				j = FIN_ES_INITIAL_POS[i][0];
				k = 24 - FIN_ES_INITIAL_POS[i][1];
				break;
			}
			case FIN_WN: {
				j = 16 - FIN_ES_INITIAL_POS[i][0];
				k = 24 - FIN_ES_INITIAL_POS[i][1];
				break;
			}
			default: {
				j = 0;
				k = 0;
				//error
				break;
			}
			}
			checker[j][k] = p.id;
			p.pawnList[i] = std::make_pair(j, k);
			const FTransform SpawnLocAndRotation(randomRotator(), getLocation(j, k), FVector3d(1.0, 1.0, 1.0));
			switch (ballMode) {
				case 0: p.ballList[i] = mainWorld->SpawnActorDeferred<ACBallCatEye>(ACBallCatEye::StaticClass(), SpawnLocAndRotation); break;
				default:p.ballList[i] = mainWorld->SpawnActorDeferred<ACBallPure>(ACBallPure::StaticClass(), SpawnLocAndRotation); break;
			}
			//p.ballList[i] = mainWorld->SpawnActorDeferred<ACBall>(ACBall::StaticClass(), SpawnLocAndRotation);
			p.ballList[i]->setType(ballMode);
			p.ballList[i]->initColor(col);
			p.ballList[i]->FinishSpawning(SpawnLocAndRotation);
			//p.ballList[i] = mainWorld->SpawnActor<ACBall>(getLocation(j, k), randomRotator());
		}
		p.color = col;
	}
}

void updateTree() {
	if (lastStep == EMPTY_STEP) return;
	if (allPlayer.hasCPUPlayer == false) return;
	thinkTree* target = root;
	for (thinkTree* p : root->move) {
		if (p->step == lastStep) target = p;
		else delete p;
	}
	root = target;
}

step makeStep(const int i, const int j, const int i2, const int j2) {
	step p;
	p.start = std::make_pair(i, j);
	p.end = std::make_pair(i2, j2);
	return p;
}

step makeStep(const pos& a, const pos& b) {
	step p;
	p.start = a;
	p.end = b;
	return p;
}

inline bool outOfBoard(const int i, const int j) {
	return i < 0 || i > 16 || j < 0 || j > 24 || checker[i][j] == UNDEFINED || (i + j) % 2 ==1;
}

inline bool inDFSVector(const int i, const int j) {
	return std::find(dfsVector.begin(), dfsVector.end(), std::make_pair(i, j)) != dfsVector.end();
}
void dfs(const int i, const int j, const int si, const int sj, thinkTree* tr, const PLAYER_ID nowMoveId, const int nowMoveFinishedPawn, const int pawni) {
	int i2 = i;
	int j2 = j;
	const int parent = tr->move.size() - 1;
	for (int k = 0; k < 6; ++k) {
		i2 = i; j2 = j;
		for (int b = 1; b < 40; ++b) {
			i2 += SINGLE_MOVE[k][0];
			j2 += SINGLE_MOVE[k][1];
			if (outOfBoard(i2, j2)) break;
			if (hasChess(i2, j2) && !(i2 == si && j2 == sj)) {
				const int i3 = i2 + b * SINGLE_MOVE[k][0];
				const int j3 = j2 + b * SINGLE_MOVE[k][1];
				if (outOfBoard(i3, j3)) break;
				if (checker[i3][j3] != EMPTY) break;
				if (inDFSVector(i3, j3)) break;
				bool validFlag = true;
				for (int q = 1; q < b; ++q) {
					const int i4 = i2 + q * SINGLE_MOVE[k][0];
					const int j4 = j2 + q * SINGLE_MOVE[k][1];
					if (hasChess(i4, j4)) {
						validFlag = false;
						break;
					}
				}
				if (validFlag) {
					dfsVector.emplace_back(std::make_pair(i3, j3));
					const int moveScore = getScore(std::make_pair(si, sj), std::make_pair(i3, j3), nowMoveId);
					tr->move.emplace_back(new thinkTree(nowMoveId, moveScore, makeStep(si, sj, i3, j3), pawni));
					if (si == i && sj == j)
						tr->move.back()->parent = -1;
					else
						tr->move.back()->parent = parent;
					dfs(i3, j3, si, sj, tr, nowMoveId, nowMoveFinishedPawn, pawni);
				}
				break;
			}
		}
	}

}

void dfsChess(const int i, const int j, thinkTree* tr, const PLAYER_ID nowMoveId, const int nowMoveFinishedPawn, const int pawni) {
	dfsVector.clear();
	dfsVector.emplace_back(std::make_pair(i, j));
	dfs(i, j, i, j, tr, nowMoveId, nowMoveFinishedPawn, pawni);
}

void movePawn(const pos& a, const pos& b, const PLAYER_ID nowMoveId, const int nowP, const int pawni) {
	checker[a.first][a.second] = EMPTY;
	checker[b.first][b.second] = nowMoveId;
	allPlayer.m[nowP].pawnList[pawni] = b;
	if (goInFinishArea(nowMoveId, a, b)) {
		++allPlayer.m[nowP].pawnFinished;
	}
	else if (goOutFinishArea(nowMoveId, a, b)) {
		--allPlayer.m[nowP].pawnFinished;
	}
}
inline void movePawn(const step& st, const PLAYER_ID nowMoveId, const int nowP, const int pawni) {
	movePawn(st.start, st.end, nowMoveId, nowP, pawni);
}
inline void movePawnInverse(const step& st, const PLAYER_ID nowMoveId, const int nowP, const int pawni) {
	movePawn(st.end, st.start, nowMoveId, nowP, pawni);
}

thinkTreeRes updateTreeGetBestStepForId(const PLAYER_ID id, thinkTree* tr, const int nowP, const int depth) {
	if (depth >= TREE_DEPTH) return std::make_tuple(NO_DEPTH, 0, 0);
	const int nowMoveFinishedPawn = allPlayer.m[nowP].pawnFinished;
	if (nowMoveFinishedPawn == 10) {
		if (allPlayer.allFinished()) {
			return std::make_tuple(NO_DEPTH, 0, 0);
		}
		return updateTreeGetBestStepForId(id, tr, allPlayer.getNextPlayer(nowP), depth + 1);
	}
	int maxi = 0;
	int maxScore = -9999999;
	int minDepth = NO_DEPTH;
	const PLAYER_ID nowMoveId = allPlayer.m[nowP].id;
	if (id != nowMoveId) {
		if (tr->move.empty()) {
			for (int qq = 0; qq < allPlayer.m[nowP].pawnList.size(); ++qq) {
				const int i = allPlayer.m[nowP].pawnList[qq].first;
				const int j = allPlayer.m[nowP].pawnList[qq].second;
				for (int k = 0; k < 6; ++k) {
					const int i2 = i + SINGLE_MOVE[k][0];
					const int j2 = j + SINGLE_MOVE[k][1];
					if (outOfBoard(i2, j2)) continue;
					if (checker[i2][j2] == EMPTY) {
						const int moveScore = getScore(std::make_pair(i, j), std::make_pair(i2, j2), nowMoveId);
						const int moveIndex = tr->move.size();
						const step st = makeStep(i, j, i2, j2);
						tr->move.emplace_back(new thinkTree(nowMoveId, moveScore, st, qq));
						if (nowMoveFinishedPawn == 9 && goInFinishArea(nowMoveId, st)) {
							movePawn(st, nowMoveId, nowP, qq);
							const auto sc = updateTreeGetBestStepForId(id, tr->move.back(), allPlayer.getNextPlayer(nowP), depth + DEPTH_FACTOR_OTHER);
							movePawnInverse(st, nowMoveId, nowP, qq);
							return sc;
						}
						if (moveScore > maxScore) {
							maxi = moveIndex;
							maxScore = moveScore;
						}
					}
				}
				const int nowMoveSize = tr->move.size();
				dfsChess(i, j, tr, nowMoveId, nowMoveFinishedPawn, qq);
				for (int q = nowMoveSize; q < tr->move.size(); ++q) {
					const int moveScore = tr->move[q]->score;
					const step& st = tr->move[q]->step;
					if (nowMoveFinishedPawn == 9 && goInFinishArea(nowMoveId, st)) {
						movePawn(st, nowMoveId, nowP, tr->move[q]->pawni);
						const auto sc = updateTreeGetBestStepForId(id, tr->move[q], allPlayer.getNextPlayer(nowP), depth + DEPTH_FACTOR_OTHER);
						movePawnInverse(st, nowMoveId, nowP, tr->move[q]->pawni);
						return sc;
					}
					if (moveScore > maxScore) {
						maxi = q;
						maxScore = moveScore;
					}
				}
			}
		}
		else {
			for (int i = 0; i < tr->move.size(); ++i) {
				const step& st = tr->move[i]->step;
				if (nowMoveFinishedPawn == 9 && goInFinishArea(nowMoveId, st)) {
					movePawn(st, nowMoveId, nowP, tr->move[i]->pawni);
					const auto sc = updateTreeGetBestStepForId(id, tr->move[i], allPlayer.getNextPlayer(nowP), depth + DEPTH_FACTOR_OTHER);
					movePawnInverse(st, nowMoveId, nowP, tr->move[i]->pawni);
					return sc;
				}
				if (tr->move[i]->score > maxScore) {
					maxScore = tr->move[i]->score;
					maxi = i;
				}
			}
		}
		const step& st = tr->move[maxi]->step;
		movePawn(st, nowMoveId, nowP, tr->move[maxi]->pawni);
		const auto sc = updateTreeGetBestStepForId(id, tr->move[maxi], allPlayer.getNextPlayer(nowP), depth + DEPTH_FACTOR_OTHER);
		movePawnInverse(st, nowMoveId, nowP, tr->move[maxi]->pawni);
		return sc;
	}
	else {
		if (tr->move.empty()) {
			for (int qq = 0; qq < allPlayer.m[nowP].pawnList.size(); ++qq) {
				const int i = allPlayer.m[nowP].pawnList[qq].first;
				const int j = allPlayer.m[nowP].pawnList[qq].second;
				for (int k = 0; k < 6; ++k) {
					const int i2 = i + SINGLE_MOVE[k][0];
					const int j2 = j + SINGLE_MOVE[k][1];
					if (outOfBoard(i2, j2)) continue;
					if (checker[i2][j2] == EMPTY) {
						const int moveScore = getScore(std::make_pair(i, j), std::make_pair(i2, j2), nowMoveId);
						const int moveIndex = tr->move.size();
						const step st = makeStep(i, j, i2, j2);
						tr->move.emplace_back(new thinkTree(nowMoveId, moveScore, st, qq));
						if (nowMoveFinishedPawn == 9 && goInFinishArea(nowMoveId, st)) {
							return std::make_tuple(depth, moveIndex, 0);
						}
						movePawn(st, nowMoveId, nowP, qq);
						const auto sc = updateTreeGetBestStepForId(id, tr->move[moveIndex], allPlayer.getNextPlayer(nowP), depth + DEPTH_FACTOR_SELF);
						movePawnInverse(st, nowMoveId, nowP, qq);
						const int score = static_cast<int>(std::get<RES_SCORE>(sc) * SCORE_FACTOR) + moveScore;
						if (std::get<RES_DEPTH>(sc) != NO_DEPTH) {
							if (std::get<RES_DEPTH>(sc) < minDepth) {
								minDepth = std::get<RES_DEPTH>(sc);
								maxi = moveIndex;
							}
						}
						else {
							if (minDepth == NO_DEPTH && score > maxScore) {
								maxi = moveIndex;
								maxScore = score;
							}
						}
					}
				}
				const int nowMoveSize = tr->move.size();
				dfsChess(i, j, tr, nowMoveId, nowMoveFinishedPawn, qq);
				for (int q = nowMoveSize; q < tr->move.size(); ++q) {
					const step& st = tr->move[q]->step;
					if (nowMoveFinishedPawn == 9 && goInFinishArea(nowMoveId, st)) {
						return std::make_tuple(depth, q, 0);
					}
					const int moveScore = tr->move[q]->score;
					movePawn(st, nowMoveId, nowP, tr->move[q]->pawni);
					const auto sc = updateTreeGetBestStepForId(id, tr->move[q], allPlayer.getNextPlayer(nowP), depth + DEPTH_FACTOR_SELF);
					movePawnInverse(st, nowMoveId, nowP, tr->move[q]->pawni);
					const int score = static_cast<int>(std::get<RES_SCORE>(sc) * SCORE_FACTOR) + moveScore;
					if (std::get<RES_DEPTH>(sc) != NO_DEPTH) {
						if (std::get<RES_DEPTH>(sc) < minDepth) {
							minDepth = std::get<RES_DEPTH>(sc);
							maxi = q;
						}
					}
					else {
						if (minDepth == NO_DEPTH && score > maxScore) {
							maxi = q;
							maxScore = score;
						}
					}
				}
			}
		}
		else {
			for (int i = 0; i < tr->move.size(); ++i) {
				const step& st = tr->move[i]->step;
				if (nowMoveFinishedPawn == 9 && goInFinishArea(nowMoveId, st)) {
					return std::make_tuple(depth, i, 0);
				}
			}
			for (int i = 0; i < tr->move.size(); ++i) {
				const step& st = tr->move[i]->step;
				const int moveScore = tr->move[i]->score;
				movePawn(st, nowMoveId, nowP, tr->move[i]->pawni);
				const auto sc = updateTreeGetBestStepForId(id, tr->move[i], allPlayer.getNextPlayer(nowP), depth + DEPTH_FACTOR_SELF);
				movePawnInverse(st, nowMoveId, nowP, tr->move[i]->pawni);
				const int score = static_cast<int>(std::get<RES_SCORE>(sc) * SCORE_FACTOR) + moveScore;
				if (std::get<RES_DEPTH>(sc) != NO_DEPTH) {
					if (std::get<RES_DEPTH>(sc) < minDepth) {
						minDepth = std::get<RES_DEPTH>(sc);
						maxi = i;
					}
				}
				else {
					if (minDepth == NO_DEPTH && score > maxScore) {
						maxi = i;
						maxScore = score;
					}
				}
			}
		}
		return std::make_tuple(minDepth, maxi, maxScore);
	}
}

void calNextStep() {
	const PLAYER_ID id = allPlayer.m[allPlayer.nowIndex].id;
	const auto ps = updateTreeGetBestStepForId(id, root, allPlayer.nowIndex, 0);
	const int bestStepIndex = std::get<RES_PATH>(ps);
	lastStep = root->move[bestStepIndex]->step;
	//coutStep(lastStep);
	/*
	if (std::get<RES_DEPTH>(ps) != NO_DEPTH) {
		std::cout << "depth:" << std::get<RES_DEPTH>(ps) << std::endl;
	}
	std::cout << "score:" << std::get<RES_SCORE>(ps) << std::endl;
	*/
	movePawn(lastStep, id, allPlayer.nowIndex, root->move[std::get<RES_PATH>(ps)]->pawni);
}
void ATiaoQi::updatePPS() {
	OurCamera->PostProcessSettings = pps;
}

void ATiaoQi::setNums(TArray<ANumberShow*> arr)
{
	nums = MoveTemp(arr);
}

ATiaoQi::ATiaoQi() {
	PrimaryActorTick.bCanEverTick = true;
	pps.AutoExposureBias = -1.0;
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent->SetMobility(EComponentMobility::Static);
	OurCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("OurCamera"));
	selectCircle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("selectCircle"));
	playerTriangle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("playerTriangle"));
	boardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("boardMesh"));
	boardMesh->SetMobility(EComponentMobility::Movable);
	OurCamera->SetupAttachment(RootComponent);
	OurCamera->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	OurCamera->PostProcessSettings = pps;
	OurCamera->PostProcessBlendWeight = 1.0;
	//OurCamera->PostProcessSettings.AutoExposureBias = -1.5;
	OurCamera->SetMobility(EComponentMobility::Movable);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneAsset(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
	static ConstructorHelpers::FObjectFinder<UMaterial> PlaneMaterial(TEXT("/Script/Engine.Material'/Game/Material/selectBall.selectBall'"));
	static ConstructorHelpers::FObjectFinder<UMaterial> TrianMaterial(TEXT("/Script/Engine.Material'/Game/Material/nowPlayerMaterial.nowPlayerMaterial'"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> board1Asset(TEXT("/Script/Engine.StaticMesh'/Game/Mesh/board1.board1'"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> board2Asset(TEXT("/Script/Engine.StaticMesh'/Game/Mesh/board2.board2'"));
	for (int i = 0; i < 5; ++i) {
		ConstructorHelpers::FObjectFinder<USoundWave> jumpSoundObject(*jumpSound[i]);
		ConstructorHelpers::FObjectFinder<USoundWave> buttonSoundObject(*buttonSound[i]);
		jumpSoundWave[i] = jumpSoundObject.Object;
		buttonSoundWave[i] = buttonSoundObject.Object;
	}
	for (int i = 0; i < 6; ++i) {
		ConstructorHelpers::FObjectFinder<UStaticMesh> meshObject(*numberMeshString[i]);
		numMesh[i] = meshObject.Object;
	}
	board1Mesh = board1Asset.Object;
	board2Mesh = board2Asset.Object;
	triangleStaticMaterial = TrianMaterial.Object;
	selectCircle->SetStaticMesh(PlaneAsset.Object);
	selectCircle->SetMaterial(0, PlaneMaterial.Object);
	playerTriangle->SetStaticMesh(PlaneAsset.Object);
	const double selectCircleSize = 0.5;
	selectCircle->SetWorldScale3D(FVector(selectCircleSize, selectCircleSize, selectCircleSize));
	selectCircle->SetVisibility(false);
	playerTriangle->SetVisibility(false);
	const double playerTriangleSize = 2.4;
	playerTriangle->SetWorldScale3D(FVector(playerTriangleSize, playerTriangleSize, playerTriangleSize));
}

void ATiaoQi::drawTrace(const int i) {
	const int j = root->move[i]->parent;
	if (j != -1) {
		const auto l1 = getLocation(root->move[i]->step.end);
		//const auto l2 = getLocation(root->move[j]->step.end);
		//line(l1.first, l1.second, l2.first, l2.second);
		animation.frame.emplace_back(l1);
		drawTrace(root->move[i]->parent);
	}
	else {
		const auto l1 = getLocation(root->move[i]->step.end);
		//const auto l2 = getLocation(lastStep.start);
		//line(l1.first, l1.second, l2.first, l2.second);
		animation.frame.emplace_back(l1);
	}
}
void ATiaoQi::showLastStep() {
	if (lastStep == EMPTY_STEP) return;
	animation.clear();
	for (int i = 0; i < root->move.size(); ++i) {
		if (root->move[i]->step == lastStep) {
			drawTrace(i);
			break;
		}
	}
	animation.frame.emplace_back(getLocation(lastStep.start));
	animation.index = animation.frame.size() - 1;
	animation.playerIndex = allPlayer.nowIndex;
	for (int i = 0; i < allPlayer.m[allPlayer.nowIndex].pawnList.size(); ++i) {
		if (allPlayer.m[allPlayer.nowIndex].pawnList[i] == lastStep.end) {
			animation.ballPtr = allPlayer.m[allPlayer.nowIndex].ballList[i];
			break;
		}
	}
	animation.enable = true;
}

void ATiaoQi::checkThisPlayer()
{
	const auto& np = allPlayer.m[allPlayer.nowIndex];
	if (np.pawnFinished != 10) return;
	if (std::find(finSeq.begin(), finSeq.end(), allPlayer.nowIndex) != finSeq.end()) return;
	if (std::find(finSeqTemp.begin(), finSeqTemp.end(), allPlayer.nowIndex) != finSeqTemp.end()) return;
	finSeqTemp.push_back(static_cast<int>(np.id));
	const int index = finSeq.size();
	nums[allPlayer.nowIndex]->SetActorHiddenInGame(false);
	nums[allPlayer.nowIndex]->SetActorLocation(getLocation(FIN_SYMBOL_POS[np.id]) + FVector3d(0,0,30));
	nums[allPlayer.nowIndex]->num->SetStaticMesh(numMesh[index]);
	if (index == 0) {
		USoundWave* HitSound = LoadObject<USoundWave>(this, *firstSound);
		if (HitSound)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), HitSound);
		}
	}
}

void ATiaoQi::flushWinner() {
	finSeq.insert(finSeq.end(), finSeqTemp.begin(), finSeqTemp.end());
	finSeqTemp.clear();
}

void ATiaoQi::BeginPlay() {
	Super::BeginPlay();
	mainWorld = GetWorld();
	triangleMaterial = UMaterialInstanceDynamic::Create(triangleStaticMaterial, this);
	playerTriangle->SetMaterial(0, triangleMaterial);
	seq.clear();
	loadGameSave();
	setSkin(tiaoqiMode);
	setBallSkin(ballMode);
	selectPlayer();
}
void ATiaoQi::setBallSkin(int typ) {
	ballMode = typ;
	if(middleBall) middleBall->Destroy();
	const FTransform SpawnLocAndRotation(randomRotator(), getLocation(8, 12), FVector3d(1.0, 1.0, 1.0));
	switch (ballMode) {
	case 0: middleBall = mainWorld->SpawnActorDeferred<ACBallCatEye>(ACBallCatEye::StaticClass(), SpawnLocAndRotation); break;
	default:middleBall = mainWorld->SpawnActorDeferred<ACBallPure>(ACBallPure::StaticClass(), SpawnLocAndRotation); break;
	}
	middleBall->setType(ballMode);
	middleBall->initColor(ColorConst::randomColor());
	middleBall->FinishSpawning(SpawnLocAndRotation);
}

FVector ATiaoQi::getMouseWorldLocation()
{
	APlayerController* playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	FVector3d mouseWorldLocation, mouseWorldDirection;
	playerController->DeprojectMousePositionToWorld(mouseWorldLocation, mouseWorldDirection);
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT(mouseWorldLocation));
	AActor* MyOwner = GetOwner();
	const FVector LineStart = MyOwner->K2_GetActorLocation();
	//const FRotator StartRotator = mouseWorldDirection.Rotation();
	const double factor = (LineStart[2] / mouseWorldDirection[2]);
	const FVector LineEnd = LineStart - mouseWorldDirection * factor;
	return LineEnd;
	//const FVector LineEnd = LineStart + StartRotator.Vector() * 1000;
	//FHitResult Hit;
	//FCollisionQueryParams QueryParams;
	//QueryParams.AddIgnoredActor(MyOwner);
	//QueryParams.AddIgnoredActor(this);
	//GetWorld()->LineTraceSingleByChannel(Hit, LineStart, LineEnd, ECC_Visibility, QueryParams);
	//DrawDebugLine(GetWorld(), LineStart, LineEnd, FColor::Red, true, 1, 0, 5);
}
void ATiaoQi::MouseClick() {
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT(" Hello "));
	if (!gameStarted) {
		const FVector3d mousePoint = getMouseWorldLocation();
		const pos p = mouseToPos(mousePoint[0], mousePoint[1]);
		if (!validPos(p)) {
			notifySelectPlayer(nullptr);
		}
		return;
	}
	auto& np = allPlayer.m[allPlayer.nowIndex];
	if (np.cpu == HUMAN_PLAYER) {
		const FVector3d mousePoint = getMouseWorldLocation();
		const pos p = mouseToPos(mousePoint[0], mousePoint[1]);
		//coutVector(mousePoint);
		//coutPos(p);
		if (validPos(p)) {
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT(" Valid Pos "));
			if (np.nowSelectPawn == -1) { //没有选中的珠子
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Select new"));
				if (checker[p.first][p.second] != np.id) return;
				for (int i = 0; i < np.pawnList.size(); ++i) {
					if (p == np.pawnList[i]) {
						np.nowSelectPawn = i;
						break;
					}
				}
				if (np.nowSelectPawn == -1) {
					return;
				}
				else {
					selectCircle->SetVisibility(true);
					selectCircle->SetWorldLocation(getLocation(p), false);
					np.oriPos = p;
					np.lastStepType = INVALID_MOVE;
				}
			}
			else {  //已经有选中的珠子
				if (checker[p.first][p.second] == EMPTY) {//点空坐标
					if (np.oriPos == p) {  //珠子回到原点
						const pos k = np.pawnList[np.nowSelectPawn];
						np.ballList[np.nowSelectPawn]->SetActorLocation(getLocation(p), false);
						selectCircle->SetWorldLocation(getLocation(p), false);
						movePawn(makeStep(k, p), np.id, allPlayer.nowIndex, np.nowSelectPawn);
						np.lastStepType = INVALID_MOVE;
						return;
					}
					const STEP st = validSingleStep(makeStep(np.pawnList[np.nowSelectPawn], p));
					if (st == INVALID_MOVE) return;
					if ((st==ONE_MOVE && np.lastStepType == INVALID_MOVE) || (st == JUMP_MOVE && np.lastStepType != ONE_MOVE)) {
						//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT(" Valid Move"));
						const pos k = np.pawnList[np.nowSelectPawn];
						np.ballList[np.nowSelectPawn]->SetActorLocation(getLocation(p), false);
						selectCircle->SetWorldLocation(getLocation(p), false);
						movePawn(makeStep(k, p), np.id, allPlayer.nowIndex, np.nowSelectPawn);
						np.lastStepType = st;
						UGameplayStatics::PlaySoundAtLocation(this, jumpSoundWave[FMath::RandRange(0, 4)], mousePoint);
						return;
					}
					ErrorSound(); // 发出非法提示音函数

				}
				else {   //点有球的坐标
					if (checker[p.first][p.second] != np.id) return;
					if (np.pawnList[np.nowSelectPawn] == p) {
						if (np.lastStepType == INVALID_MOVE) { //选中球但没有移动，再次点选球来取消
							selectCircle->SetVisibility(false);
							np.nowSelectPawn = -1;
						}
						return;
					}
					else {
						if (np.lastStepType == INVALID_MOVE) {
							int nowSelectPawn = -1;
							for (int i = 0; i < np.pawnList.size(); ++i) {
								if (p == np.pawnList[i]) {
									nowSelectPawn = i;
									break;
								}
							}
							if (nowSelectPawn == -1) {
								selectCircle->SetVisibility(false);
								np.nowSelectPawn = -1;
								return;
							}
							else {//点击一颗珠子后点击别的珠子
								np.nowSelectPawn = nowSelectPawn;
								selectCircle->SetVisibility(true);
								selectCircle->SetWorldLocation(getLocation(p), false);
								np.oriPos = p;
							}
						}
					}
				}
			}
		}
	}
}

void ATiaoQi::ChangePlayerSymbol() {
	const auto& np = allPlayer.m[allPlayer.nowIndex];
	playerTriangle->SetWorldLocation(getLocation(PLAYER_ADDER_INITIAL_POS[np.id]) + FVector3d(0,0,1), false);
	triangleMaterial->SetVectorParameterValue(FName(TEXT("Color")), np.color);
	//playerTriangle->SetMaterial(0, triangleMaterial);
	playerTriangle->SetRelativeRotation(FRotator(0, 180.0 + ROTATION_ID[np.id], 0));
}

void ATiaoQi::NextStep() {
	if (gameStarted == false) return;
	if (animation.enable) {
		//animation.goNextStep = true;
		return;
	}
	if (allPlayer.getNowPlayer().pawnFinished == 10) {
		if (allPlayer.allFinished()) {
			widget->allFinish();
			return;
		}
		else {
			bool allHumanFinish = true;
			for (const auto& np:allPlayer.m) {
				if (np.cpu == HUMAN_PLAYER) {
					if (np.pawnFinished < 10) {
						allHumanFinish = false;
						break;
					}
				}
			}
			if (allHumanFinish) {
				widget->humanFinish();
			}
		}
	}
	const int lastIndex = allPlayer.nowIndex;
	if(allPlayer.m[allPlayer.nowIndex].cpu == CPU_PLAYER){
		calNextStep();
		animation.goNextStep = true;
		showLastStep();
		updateTree();
		checkThisPlayer();
		if (allPlayer.nextPlayer()) {
			flushWinner();
		}
		if (allPlayer.allFinished()) return;
	}
	else {  //human player press next step
		if (allPlayer.m[allPlayer.nowIndex].lastStepType == INVALID_MOVE) return;
		lastStep = makeStep(allPlayer.m[allPlayer.nowIndex].oriPos, allPlayer.m[allPlayer.nowIndex].pawnList[allPlayer.m[allPlayer.nowIndex].nowSelectPawn]);
		allPlayer.m[allPlayer.nowIndex].lastStepType = INVALID_MOVE;
		allPlayer.m[allPlayer.nowIndex].nowSelectPawn = -1;
		selectCircle->SetVisibility(false);
		updateTree();
		checkThisPlayer();
		if (allPlayer.nextPlayer()) {
			flushWinner();
		}
		USoundWave* HitSound = LoadObject<USoundWave>(this, *nextStepSound[FMath::RandRange(0,4)]); //Human next step sound
		if (HitSound)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), HitSound);
		}
		if (allPlayer.allFinished()) return;
		ChangePlayerSymbol();
		if (allPlayer.getNowPlayer().cpu == CPU_PLAYER) {
			NextStep();
		}
	}
}
void ATiaoQi::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	if (animation.enable) {
		addTime(DeltaTime);
	}
	else {
		if (animation.goNextStep) {
			animation.goNextStep = false;
			ChangePlayerSymbol();
			NextStep();
		}
	}
}

// Called to bind functionality to input
void ATiaoQi::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Mouse_Click", IE_Released, this, &ATiaoQi::MouseClick);
	PlayerInputComponent->BindAction("Next_Step", IE_Released, this, &ATiaoQi::NextStep);
	PlayerInputComponent->BindAction("Next_Step", IE_Released, this, &ATiaoQi::NextStep);
	PlayerInputComponent->BindAxis("CameraHeight", this, &ATiaoQi::CameraHeight);
}
void ATiaoQi::CameraHeight(float Value) {
	const auto loc = OurCamera->GetRelativeLocation();
	OurCamera->SetRelativeLocation(loc + Value * FVector3d(0, 0, 20));
}
void ATiaoQi::selectPlayer() {
	for (size_t i = 0; i < 6; ++i) {
		playerAdderArray[i] = mainWorld->SpawnActor<AplayerAdder>(getLocation(PLAYER_ADDER_INITIAL_POS[i]) + FVector3d(0,0,30), FRotator::ZeroRotator);
		playerAdderArray[i]->setParent(this);
		playerAdderArray[i]->setIndex(i);
		playerAdderArray[i]->setColor(ColorConst::COLOR_ARRAY[i]);
	}
}
void ATiaoQi::notifySelectPlayer(AplayerAdder* originSignal) {
	int chooseI = -1;
	int removeI = -1;
	for (int i = 0; i < 6; ++i) {
		if (playerAdderArray[i] == originSignal) {
			chooseI = i;
			break;
		}
	}
	for (int i = 0; i < 6;++i) {
		if (playerAdderArray[i]->noClick(originSignal)) {
			removeI = i;
		}
	}
	if (removeI != -1) {
		auto iter = std::find(seq.begin(), seq.end(), removeI);
		if (iter != seq.end()) {
			seq.erase(iter);
		}
	}
	if (chooseI == -1 && removeI == -1) return;
	if (chooseI != -1) seq.emplace_back(chooseI);
	for (int i = 0; i < seq.size(); ++i) {
		nums[i]->SetActorHiddenInGame(false);
		nums[i]->SetActorLocation(getLocation(NUM_INITIAL_POS[seq[i]]) + FVector3d(0,0,100));
	}
	for (int i = seq.size(); i < 6; ++i) {
		nums[i]->SetActorHiddenInGame(true);
	}
}

void ATiaoQi::setWidget(UTiaoQiWidget* w) {
	widget = w;
}

void ATiaoQi::setSkin(int typ) {
	tiaoqiMode = typ;
	switch (tiaoqiMode) {
	case 0: {
		boardMesh->SetStaticMesh(board1Mesh);
		boardMesh->SetWorldLocation(FVector3d(0, 0, 0));
		break;
	}
	case 1: {
		boardMesh->SetStaticMesh(board2Mesh);
		boardMesh->SetWorldLocation(FVector3d(0, 0, -10));
		break;
	}
	default: {
		boardMesh->SetStaticMesh(board1Mesh);
		boardMesh->SetWorldLocation(FVector3d(0, 0, 0));
		break;
	}
	}
}
void ATiaoQi::saveGame() {
	if (UMySaveGame* SaveGameInstance = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass()))){
		SaveGameInstance->tiaoqiMode = tiaoqiMode;
		SaveGameInstance->ballMode = ballMode;
		// 即时保存游戏。
		if (UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotNameString, UserIndex))
		{
			// 成功保存。
		}
	}
}
void ATiaoQi::ErrorSound() {
	USoundWave* HitSound = LoadObject<USoundWave>(this, TEXT("/Script/Engine.SoundWave'/Game/soundFX/Error/magnet_start.magnet_start'"));
	if (HitSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), HitSound);
	}
}
int ATiaoQi::gameStart() {  // 0 成功 1 失败 2成功但没有人类玩家
	if (gameStarted) return 1;
	if (seq.size() < 2) {
		ErrorSound();
		return 1;
	}
	allPlayer.clear();
	bool hasHumanPlayer = false;
	for (int i = 0; i < seq.size(); ++i) {
		switch (playerAdderArray[seq[i]]->selectOne) {
		case AplayerAdder::SELECT::NONE:break;
		case AplayerAdder::SELECT::CPU: {
			allPlayer.hasCPUPlayer = true;
			allPlayer.addPlayer(static_cast<PLAYER_ID>(seq[i]), true);
			break;
		}
		case AplayerAdder::SELECT::HUMAN: {
			hasHumanPlayer = true;
			allPlayer.addPlayer(static_cast<PLAYER_ID>(seq[i]), false);
			break;
		}
		default:break;
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT(" Game start "));
	saveGame();
	USoundWave* gameStartSound = LoadObject<USoundWave>(this, TEXT("/Script/Engine.SoundWave'/Game/soundFX/GameStart/anchor_action.anchor_action'"));
	if (gameStartSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), gameStartSound);
	}
	if (middleBall) middleBall->Destroy();
	initCheckers(ballMode);
	gameStarted = true;
	playerTriangle->SetVisibility(true);
	for (auto* np : nums) {
		np->SetActorHiddenInGame(true);
	}
	for (size_t i = 0; i < 6; ++i) {
		playerAdderArray[i]->remove();
	}
	ChangePlayerSymbol();
	if (allPlayer.m[allPlayer.nowIndex].cpu == CPU_PLAYER) {
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT(" First is CPU "));
		NextStep();
	}
	return hasHumanPlayer ? 0 : 2;
	
}
