#include "GameOverScene.h"
#include "SoundManager.h"

using namespace KamataEngine;

GameOverScene::~GameOverScene() { delete bgSprite_; }

void GameOverScene::Initialize() {
	isFinished_ = false;

	// ゲームオーバー画像のロード（ファイル名は画像素材に合わせて変更してください）
	bgTextureHandle_ = TextureManager::Load("GameOver.png");

	if (!bgSprite_) {
		bgSprite_ = Sprite::Create(bgTextureHandle_, {0.0f, 0.0f});
	} else {
		bgSprite_->SetTextureHandle(bgTextureHandle_);
	}

	bgSprite_->SetSize({1280.0f, 720.0f});
	SoundManager::GetInstance()->PlayBGM("NormalBGM");
}

void GameOverScene::Update() {
	Input* input = Input::GetInstance();
	if (input->TriggerKey(DIK_SPACE)) {
		SoundManager::GetInstance()->PlaySE("SceanTransition", 0.5f);
		isFinished_ = true;
	}
}

void GameOverScene::Draw() {
	Sprite::PreDraw();

	if (bgSprite_) {
		bgSprite_->Draw();
	}

	Sprite::PostDraw();
}