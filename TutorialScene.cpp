#include "TutorialScene.h"
#include "SoundManager.h"

TutorialScene::~TutorialScene() {
	delete sprite_;
	delete fade_;
}

void TutorialScene::Initialize() {
	SoundManager::GetInstance()->PlayBGM("Tutorial", true, 0.3f);
	isFinished_ = false;

	// 画像読み込み
	// textureHandle_ = KamataEngine::TextureManager::Load("tutorial.png");
	sprite_ = KamataEngine::Sprite::Create(textureHandle_, {0.0f, 0.0f});

	// フェード初期化＆フェードイン開始
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Status::FadeIn, 1.5f); 

	phase_ = Phase::kFadeIn;
}

void TutorialScene::Update() {
	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}
		break;

	case Phase::kMain:
		// SPACEキー または ENTERキー が押されたらフェードアウトへ移行
		if (KamataEngine::Input::GetInstance()->TriggerKey(DIK_SPACE) || KamataEngine::Input::GetInstance()->TriggerKey(DIK_RETURN)) {
			SoundManager::GetInstance()->PlaySE("Select", 0.5f);
			fade_->Start(Status::FadeOut, 1.5f); // ★ Fade::Status:: に修正
			phase_ = Phase::kFadeOut;
		}
		break;

	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished()) {
			isFinished_ = true;
		}
		break;
	}
}

void TutorialScene::Draw() {
	// 1. チュートリアル一枚絵の描画
	KamataEngine::Sprite::PreDraw();

	if (sprite_) {
		sprite_->Draw();
	}

	KamataEngine::Sprite::PostDraw();

	// 2. 最手前にフェードを描画
	if (fade_) {
		fade_->Draw();
	}
}