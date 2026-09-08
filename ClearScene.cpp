#include "ClearScene.h"
#include "SoundManager.h"

ClearScene::~ClearScene() {
	delete backgroundSprite_;
	delete spaceSprite_;
}

void ClearScene::Initialize() {
	// フラグ初期化
	isFinished_ = false;

	// シーン開始後、30フレーム（約0.5秒）はキー入力を受け付けない
	inputWaitTimer_ = 30;

	SoundManager::GetInstance()->PlayBGM("Clear", true, 0.3f);

	// 背景画像の読み込みとスプライト生成
	// textureHandleBG_ = TextureManager::Load("result_bg.png");
	backgroundSprite_ = KamataEngine::Sprite::Create(textureHandleBG_, {0.0f, 0.0f});

	// UI画像の読み込みと配置
	// textureHandleSpace_ = TextureManager::Load("spaceToTitle.png");
	spaceSprite_ = KamataEngine::Sprite::Create(textureHandleSpace_, {0.0f, 0.0f});

	spaceSprite_->SetSize({300.0f, 40.0f});
	spaceSprite_->SetPosition({(1280.0f - 300.0f) / 2.0f, 620.0f});
}

void ClearScene::Update() {
	// タイマー消化中はキー入力を受け付けずに処理を抜ける
	if (inputWaitTimer_ > 0) {
		--inputWaitTimer_;
		return;
	}

	// SPACEキーでシーン終了（タイトルへ戻る等のトリガー）
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		SoundManager::GetInstance()->PlaySE("Select", 0.5f);

		// SceneManager が全自動で暗転 ➔ 次のシーン移行 ➔ 明転
		isFinished_ = true;
	}
}

void ClearScene::Draw() {
	// 2Dスプライト描画前処理
	Sprite::PreDraw();

	// 背景画像を描画
	if (backgroundSprite_) {
		backgroundSprite_->Draw();
	}

	// UIスプライトを描画
	if (spaceSprite_) {
		spaceSprite_->Draw();
	}

	Sprite::PostDraw();
}