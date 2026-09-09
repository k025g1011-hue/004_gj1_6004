#define NOMINMAX
#include "Player.h"
#include "MapChipField.h"
#include <algorithm>

using namespace KamataEngine;

void Player::Initialize(const PlayerTextureHandles& handles) {
	textures_ = handles;
	// 初期テクスチャは右歩きを設定（切り出し位置指定で表示）
	sprite_ = Sprite::Create(textures_.walkRight, {0.0f, 0.0f});
	if (sprite_) {
		sprite_->SetTextureRect({0.0f, 0.0f}, {kFrameWidth, kFrameHeight});
	}
	Reset({200.0f, 500.0f});
}

void Player::Reset(const Vector2& position) {
	position_ = position;
	velocity_ = {};
	facing_ = 1;
	onGround_ = false;
	dashTimer_ = 0;
	usedAirDash_ = false;
	hp_ = kMaxHp;
	invincibleTimer_ = 0;
	invincibleJustEnded_ = false;

	state_ = MotionState::kIdle;
	animTimer_ = 0;
	currentFrame_ = 0;
	attackTimer_ = 0;

	UpdateAnimation();
}

void Player::SetMapBounds(float left, float right) {
	mapLeft_ = left;
	mapRight_ = right;
}

void Player::SetInvincible(int frames) {
	if (frames > invincibleTimer_) {
		invincibleTimer_ = frames;
	}
	invincibleJustEnded_ = false;
}

Vector2 Player::GetCenter() const { return {position_.x + size_.x * 0.5f, position_.y + size_.y * 0.5f}; }

AABB2 Player::GetAABB() const {
	AABB2 aabb;
	aabb.min = position_;
	aabb.max = {position_.x + size_.x, position_.y + size_.y};
	return aabb;
}

void Player::OnDamaged() {
	if (invincibleTimer_ > 0 || dashTimer_ > 0 || IsDead()) {
		return;
	}
	--hp_;
	invincibleTimer_ = kInvincibleDuration;
	invincibleJustEnded_ = false;
	velocity_.y = -6.0f;
	velocity_.x = static_cast<float>(-facing_) * 4.0f;

	if (IsDead()) {
		state_ = MotionState::kDead;
		animTimer_ = 0;
		currentFrame_ = 0;
	}
}

void Player::TriggerAttack() {
	if (IsDead() || attackTimer_ > 0) {
		return;
	}
	state_ = MotionState::kAttack;
	attackTimer_ = 16; // 全4コマ × 4フレーム表示 = 計16フレーム
	animTimer_ = 0;
	currentFrame_ = 0;
}

void Player::InputMove() {
	Input* input = Input::GetInstance();

	const bool pressRight = input->PushKey(DIK_RIGHT) || input->PushKey(DIK_D);
	const bool pressLeft = input->PushKey(DIK_LEFT) || input->PushKey(DIK_A);
	const bool pressJump = input->PushKey(DIK_SPACE) || input->PushKey(DIK_Z) || input->PushKey(DIK_W);
	const bool pressDash = input->TriggerKey(DIK_LSHIFT) || input->TriggerKey(DIK_X);
	const bool pressAttack = input->TriggerKey(DIK_F) || input->TriggerKey(DIK_C); // 例: FキーまたはCキーで攻撃

	if (pressAttack && attackTimer_ <= 0) {
		TriggerAttack();
	}

	if (dashTimer_ > 0) {
		--dashTimer_;
		velocity_.x = static_cast<float>(facing_) * kDashSpeed;
		velocity_.y = 0.0f;
		return;
	}

	if (pressRight) {
		velocity_.x += kAcceleration;
		facing_ = 1;
	} else if (pressLeft) {
		velocity_.x -= kAcceleration;
		facing_ = -1;
	} else {
		velocity_.x *= (1.0f - kAttenuation);
	}
	velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

	if (onGround_ && pressJump) {
		velocity_.y = kJumpSpeed;
		onGround_ = false;
	} else if (!onGround_ && !pressJump && velocity_.y < 0.0f) {
		velocity_.y *= kJumpCut;
	}

	const bool canDash = onGround_ || !usedAirDash_;
	if (pressDash && canDash) {
		dashTimer_ = kDashDuration;
		if (!onGround_) {
			usedAirDash_ = true;
		}
	}

	if (!onGround_) {
		velocity_.y += kGravity;
		if (velocity_.y > kLimitFallSpeed) {
			velocity_.y = kLimitFallSpeed;
		}
	}
}

void Player::UpdateAnimation() {
	// 1. 状態の更新
	if (IsDead()) {
		state_ = MotionState::kDead;
	} else if (attackTimer_ > 0) {
		--attackTimer_;
		state_ = MotionState::kAttack;
	} else if (std::abs(velocity_.x) > 0.5f) {
		state_ = MotionState::kWalk;
	} else {
		state_ = MotionState::kIdle;
	}

	// 2. フレームカウント・表示コマの計算
	++animTimer_;
	uint32_t activeTexture = 0;

	switch (state_) {
	case MotionState::kWalk: {
		// 8フレームごとに次のコマへ (0 -> 1 -> 2 -> 3 -> 0)
		currentFrame_ = (animTimer_ / 8) % 4;
		activeTexture = (facing_ > 0) ? textures_.walkRight : textures_.walkLeft;
		break;
	}

	case MotionState::kIdle: {
		// 待機中は「歩きテクスチャの1コマ目」を使用
		currentFrame_ = 0;
		activeTexture = (facing_ > 0) ? textures_.walkRight : textures_.walkLeft;
		break;
	}

	case MotionState::kAttack: {
		// 4フレームごとに次のコマへ (0 -> 1 -> 2 -> 3)
		currentFrame_ = std::min((16 - attackTimer_) / 4, 3);
		activeTexture = (facing_ > 0) ? textures_.attackRight : textures_.attackLeft;
		break;
	}

	case MotionState::kDead: {
		// 6フレームごとに次のコマへ（最後は8コマ目で停止）
		currentFrame_ = std::min(animTimer_ / 6, 7);
		activeTexture = (facing_ > 0) ? textures_.deadRight : textures_.deadLeft;
		break;
	}
	}

	// 3. テクスチャと切り出し範囲の適用
	if (sprite_) {
		sprite_->SetTextureHandle(activeTexture);

		// 画像シート上の切り出し左上座標（X）を計算
		float srcX = static_cast<float>(currentFrame_) * kFrameWidth;

		// スプライトシートの切り出し範囲（UV / SrcRect）を指定
		// ※ KamataEngine の仕様に合わせて SetTextureRect / SetSrcRect を呼び出します
		sprite_->SetTextureRect({srcX, 0.0f}, {kFrameWidth, kFrameHeight});
	}
}

void Player::Update(MapChipField* mapChipField) {
	invincibleJustEnded_ = false;
	if (invincibleTimer_ > 0) {
		--invincibleTimer_;
		if (invincibleTimer_ == 0) {
			invincibleJustEnded_ = true;
		}
	}

	// 死亡していない場合のみ移動操作を受け付ける
	if (!IsDead()) {
		InputMove();
	}

	position_.x += velocity_.x;
	AABB2 aabb = GetAABB();
	float resolvedX = position_.x;
	if (mapChipField && mapChipField->ResolveBlockX(aabb, resolvedX, velocity_.x)) {
		position_.x = resolvedX;
		velocity_.x = 0.0f;
	}

	if (position_.x < mapLeft_ + 8.0f) {
		position_.x = mapLeft_ + 8.0f;
		velocity_.x = 0.0f;
	}
	if (position_.x > mapRight_ - 8.0f - size_.x) {
		position_.x = mapRight_ - 8.0f - size_.x;
		velocity_.x = 0.0f;
	}

	position_.y += velocity_.y;
	aabb = GetAABB();
	float resolvedY = position_.y;
	bool landed = false;
	if (mapChipField && mapChipField->ResolveBlockY(aabb, resolvedY, velocity_.y, landed)) {
		position_.y = resolvedY;
		if (landed) {
			onGround_ = true;
			usedAirDash_ = false;
			if (velocity_.y > 0.0f) {
				velocity_.y = 0.0f;
			}
		} else {
			if (velocity_.y < 0.0f) {
				velocity_.y = 0.0f;
			}
			onGround_ = false;
		}
	} else {
		onGround_ = false;
	}

	// アニメーション計算の更新
	UpdateAnimation();
}

void Player::Draw(const Vector2& camera) {
	if (!sprite_) {
		return;
	}

	// カラー（点滅やダッシュ色の演出）設定
	if (invincibleTimer_ > 0 && (invincibleTimer_ / 2) % 2 == 0) {
		sprite_->SetColor({0.4f, 0.8f, 1.0f, 0.4f});
	} else if (dashTimer_ > 0) {
		sprite_->SetColor({1.0f, 1.0f, 0.4f, 1.0f});
	} else {
		sprite_->SetColor({1.0f, 1.0f, 1.0f, 1.0f}); // 本来のスプライトの色を表示するため白(1.0)に変更
	}

	sprite_->SetPosition({position_.x - camera.x, position_.y - camera.y});
	sprite_->SetSize(size_);
	sprite_->Draw();
}