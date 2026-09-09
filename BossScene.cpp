#include "BossScene.h"
#include "SoundManager.h"
#include <Windows.h> // OutputDebugStringA 用
#include <cstdio>    // sprintf_s 用

void BossScene::Initialize() {
	GameScene::Initialize();
	SetStage(3); // ボスステージ背景
	// ボスステージ用マップCSVを読み込んで再構築
	BuildWorld("Resources/boss_stage.csv");

	// ボス戦専用のダッシュパラメータに上書き
	player_->SetDashParams(24.0f, 18);

	SoundManager::GetInstance()->PlayBGM("BossBGM");
}

void BossScene::Update() {
	GameScene::Update();

	// ボスが存在し、かつ生存しているボスが1体もない状態かチェック
	bool bossExists = (bossA_ || bossB_);
	bool bossAAlive = (bossA_ && !bossA_->IsDead());
	bool bossBAlive = (bossB_ && !bossB_->IsDead());

	// ボスが1体以上出現しており、かつ生きているボスがいないならクリア！
	if (bossExists && !bossAAlive && !bossBAlive) {
		isFinished_ = true;
	}
}

void BossScene::Draw() {
	GameScene::Draw();

	KamataEngine::Sprite::PreDraw();
	GameScene::DrawHp(); 
	KamataEngine::Sprite::PostDraw();


}