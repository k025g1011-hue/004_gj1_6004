#include "Stage2Scene.h"
#include "SoundManager.h"

void Stage2Scene::Initialize() {
	GameScene::Initialize();
	SetStage(1); // ステージ2背景
	// Stage2用マップCSVを読み込んで再構築
	BuildWorld("Resources/stage2.csv");
	SoundManager::GetInstance()->PlayBGM("NormalBGM");
}