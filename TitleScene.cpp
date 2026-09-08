#include "TitleScene.h"
#include "SoundManager.h"

TitleScene::~TitleScene() {
	delete spaceSprite_;
	delete bgSprite_;
}

void TitleScene::Initialize() {
	// フラグ初期化（BaseSceneのメンバ変数）
	isFinished_ = false;

	SoundManager::GetInstance()->PlayBGM("Title", true, 0.2f);

	// カメラ初期化
	camera_.Initialize();

	// UI画像の読み込み・設定
	spaceSprite_ = KamataEngine::Sprite::Create(textureHandleSpace_, {0.0f, 0.0f});
	bgSprite_ = KamataEngine::Sprite::Create(bgTextureHandle_, {0.0f, 0.0f});

	spaceSprite_->SetSize({300.0f, 40.0f});
	spaceSprite_->SetPosition({(1280.0f - 300.0f) / 2.0f, 550.0f});
}

void TitleScene::Update() {
	blinkTimer_ += 1.0f / 60.0f;

	// SPACEキーが押されたら「このシーンは終わり！」と主張するだけ
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		SoundManager::GetInstance()->PlaySE("Select", 0.5f);

		// SceneManager が暗転フェードアウト ➔ 次のシーン移行 ➔ 明転フェードイン を全自動で行う
		isFinished_ = true;
	}
}

void TitleScene::Draw() {
	KamataEngine::Sprite::PreDraw();

	if (bgSprite_) {
		bgSprite_->Draw();
	}
	if (spaceSprite_) {
		spaceSprite_->Draw();
	}

	KamataEngine::Sprite::PostDraw();
}