#include "TutorialScene.h"
#include "SoundManager.h"
#include <cmath>

using namespace KamataEngine;

TutorialScene::~TutorialScene() {
	delete hintSprite_;
	hintSprite_ = nullptr;
}

void TutorialScene::Initialize() {
	GameScene::Initialize();
	SetStage(0);
	BuildWorld("Resources/Tutorial.csv");

	showPlayerHp_ = false;
	enablePlayerDamage_ = false;

	hint_ = Hint::kMove;
	wasOnGround_ = false;
	hasLanded_ = false;
	sawCinch_ = false;

	hintTextures_[static_cast<size_t>(Hint::kMove)] = TextureManager::Load("./Resources/Tutorial1.png");
	hintTextures_[static_cast<size_t>(Hint::kJump)] = TextureManager::Load("./Resources/Tutorial2.png");
	hintTextures_[static_cast<size_t>(Hint::kDashStitch)] = TextureManager::Load("./Resources/Tutorial3.png");
	hintTextures_[static_cast<size_t>(Hint::kDashStitch2)] = TextureManager::Load("./Resources/Tutorial4.png");
	hintTextures_[static_cast<size_t>(Hint::kCinch)] = TextureManager::Load("./Resources/Tutorial5.png");
	hintTextures_[static_cast<size_t>(Hint::kDoor)] = TextureManager::Load("./Resources/Tutorial6.png");

	hintSprite_ = Sprite::Create(hintTextures_[0], {kHintX, kHintY});
	hintSprite_->SetSize({kHintW, kHintH});

	SoundManager::GetInstance()->PlayBGM("NormalBGM");
}

void TutorialScene::UpdateHint() {
	if (!player_ || !hookStitch_) {
		return;
	}

	const bool onGround = player_->IsOnGround();
	if (onGround) {
		hasLanded_ = true;
	}

	const int stitch = hookStitch_->GetCount();
	if (hookStitch_->IsCinching()) {
		sawCinch_ = true;
	}

	const Hint prev = hint_;

	if (hint_ == Hint::kMove) {
		if (std::abs(player_->GetVelocity().x) > 1.5f) {
			hint_ = Hint::kJump;
		}
	} else if (hint_ == Hint::kJump) {
		// 着地したあとに、上方向へ跳んだときだけ進める
		// 落下・ダッシュ・スポーン直後の空中は無視
		if (hasLanded_ && !onGround && player_->GetVelocity().y < -8.0f && !player_->IsDashing()) {
			hint_ = Hint::kDashStitch;
		}
	} else if (hint_ == Hint::kDashStitch) {
		if (stitch >= 1) {
			hint_ = Hint::kDashStitch2;
		}
	} else if (hint_ == Hint::kDashStitch2) {
		if (stitch >= 2) {
			hint_ = Hint::kCinch;
		}
	} else if (hint_ == Hint::kCinch) {
		if (sawCinch_ || IsStageCleared()) {
			hint_ = Hint::kDoor;
		}
	}

	wasOnGround_ = onGround;
	if (hint_ != prev) {
		UpdateHintSprite();
	}
}

void TutorialScene::UpdateHintSprite() {
	const uint32_t handle = hintTextures_[static_cast<size_t>(Hint::kCount) > 0 ? static_cast<size_t>(hint_) : 0];
	delete hintSprite_;
	hintSprite_ = Sprite::Create(handle, {kHintX, kHintY});
	if (hintSprite_) {
		hintSprite_->SetSize({kHintW, kHintH});
	}
}

void TutorialScene::Update() {
	GameScene::Update();
	isGameOver_ = false;
	if (!isFinished_) {
		UpdateHint();
	}
}

void TutorialScene::Draw() {
	GameScene::Draw();

	Sprite::PreDraw();
	if (hintSprite_) {
		hintSprite_->SetRotation(0.0f);
		hintSprite_->SetPosition({kHintX, kHintY});
		hintSprite_->SetSize({kHintW, kHintH});
		hintSprite_->Draw();
	}
	Sprite::PostDraw();
}