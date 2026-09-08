#include "Stage1Scene.h"

void Stage1Scene::Initialize() {
	// 親クラス（GameScene）の共通初期化を実行（プレイヤー生成など）
	GameScene::Initialize();

	// Stage1用マップCSVを読み込んで再構築
	BuildWorld("Resources/stage1.csv");
}