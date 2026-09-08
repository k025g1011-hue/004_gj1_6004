#include "Stage3Scene.h"

void Stage3Scene::Initialize() {
	GameScene::Initialize();

	// Stage3用マップCSVを読み込んで再構築
	BuildWorld("Resources/stage3.csv");
}