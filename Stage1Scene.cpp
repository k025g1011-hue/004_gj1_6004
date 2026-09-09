#include "Stage1Scene.h"

void Stage1Scene::Initialize() {
	// 親クラス（GameScene）の共通初期化を実行（プレイヤー生成など）
	GameScene::Initialize();
	SetStage(0); // ステージ1背景
	// Stage1用マップCSVを読み込んで再構築
	BuildWorld("Resources/stage1.csv");
}