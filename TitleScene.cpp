#include "TitleScene.h"
#include "SoundManager.h"

TitleScene::~TitleScene() { delete bgSprite_; }

void TitleScene::Initialize() {
	// フラグ初期化（BaseSceneのメンバ変数）
	isFinished_ = false;

	SoundManager::GetInstance()->PlayBGM("Title", true, 0.2f);

	// カメラ初期化
	camera_.Initialize();

	// 背景テクスチャのロード
	bgTextureHandle_ = TextureManager::Load("title.png");

	// 背景スプライトの生成・初期化（1280x720）
	if (!bgSprite_) {
		bgSprite_ = KamataEngine::Sprite::Create(bgTextureHandle_, {0.0f, 0.0f});
	} else {
		bgSprite_->SetTextureHandle(bgTextureHandle_);
	}
	bgSprite_->SetSize({1280.0f, 720.0f});
}

void TitleScene::Update() {
	// SPACEキーが押されたら次のシーンへ移行
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		SoundManager::GetInstance()->PlaySE("Select", 0.5f);

		// SceneManager が暗転フェードアウト ➔ 次のシーン移行 ➔ 明転フェードイン
		isFinished_ = true;
	}
}

void TitleScene::Draw() {
	KamataEngine::Sprite::PreDraw();

	// 背景描画
	if (bgSprite_) {
		bgSprite_->Draw();
	}

	KamataEngine::Sprite::PostDraw();
}