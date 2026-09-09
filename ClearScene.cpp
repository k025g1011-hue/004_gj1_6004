#include "ClearScene.h"
#include "SoundManager.h"

ClearScene::~ClearScene() { delete bgSprite_; }

void ClearScene::Initialize() {
	// フラグ初期化
	isFinished_ = false;

	// シーン開始後、30フレーム（約0.5秒）はキー入力を受け付けない
	inputWaitTimer_ = 30;

	SoundManager::GetInstance()->PlayBGM("Clear", true, 0.3f);

	// クリア画像のロード（ファイル名は画像素材に合わせて変更してください）
	bgTextureHandle_ = TextureManager::Load("GameClear.png");

	// 背景スプライトの生成・初期化（1280x720）
	if (!bgSprite_) {
		bgSprite_ = KamataEngine::Sprite::Create(bgTextureHandle_, {0.0f, 0.0f});
	} else {
		bgSprite_->SetTextureHandle(bgTextureHandle_);
	}
	bgSprite_->SetSize({1280.0f, 720.0f});
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

	// 背景描画
	if (bgSprite_) {
		bgSprite_->Draw();
	}

	Sprite::PostDraw();
}