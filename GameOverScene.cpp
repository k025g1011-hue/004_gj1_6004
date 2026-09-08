#include "GameOverScene.h"

using namespace KamataEngine;

void GameOverScene::Initialize() {
	isFinished_ = false;
	whiteTexture_ = TextureManager::Load("white.png");
	bgSprite_ = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
}

void GameOverScene::Update() {
	// KamataEngine の Input を使用してスペースキー入力を判定
	Input* input = Input::GetInstance();
	if (input->TriggerKey(DIK_SPACE)) {
		isFinished_ = true;
	}
}

void GameOverScene::Draw() {
	Sprite::PreDraw();

	// 暗い赤色の背景を描画
	if (bgSprite_) {
		bgSprite_->SetColor({0.2f, 0.05f, 0.05f, 1.0f});
		bgSprite_->SetPosition({0.0f, 0.0f});
		bgSprite_->SetSize({1280.0f, 720.0f});
		bgSprite_->Draw();
	}

	Sprite::PostDraw();
}