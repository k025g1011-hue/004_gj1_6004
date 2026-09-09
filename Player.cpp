#define NOMINMAX
#include "Player.h"
#include "MapChipField.h"
#include "SoundManager.h"
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

	// ダッシュパラメータのリセット
	ResetDashParams();

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
	SoundManager::GetInstance()->PlaySE("Damage", 0.3f);
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
	const bool pressAttack = input->TriggerKey(DIK_F) || input->TriggerKey(DIK_C);

	if (pressAttack && attackTimer_ <= 0) {
		SoundManager::GetInstance()->PlaySE("Stitch", 0.3f);
		TriggerAttack();
	}

	if (dashTimer_ > 0) {
		--dashTimer_;
		// ★ 変数 dashSpeed_ を使用
		velocity_.x = static_cast<float>(facing_) * dashSpeed_;
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
		SoundManager::GetInstance()->PlaySE("Jump", 0.3f);
		velocity_.y = kJumpSpeed;
		onGround_ = false;
	} else if (!onGround_ && !pressJump && velocity_.y < 0.0f) {
		velocity_.y *= kJumpCut;
	}

	const bool canDash = onGround_ || !usedAirDash_;
	if (pressDash && canDash) {
		SoundManager::GetInstance()->PlaySE("Dash", 0.5f);
		// ★ 変数 dashDuration_ を使用
		dashTimer_ = dashDuration_;
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

	++animTimer_;
	uint32_t activeTexture = 0;

	switch (state_) {
	case MotionState::kWalk: {
		currentFrame_ = (animTimer_ / 8) % 4;
		activeTexture = (facing_ > 0) ? textures_.walkRight : textures_.walkLeft;
		break;
	}

	case MotionState::kIdle: {
		currentFrame_ = 0;
		activeTexture = (facing_ > 0) ? textures_.walkRight : textures_.walkLeft;
		break;
	}

	case MotionState::kAttack: {
		currentFrame_ = std::min((16 - attackTimer_) / 4, 3);
		activeTexture = (facing_ > 0) ? textures_.attackRight : textures_.attackLeft;
		break;
	}

	case MotionState::kDead: {
		currentFrame_ = std::min(animTimer_ / 6, 7);
		activeTexture = (facing_ > 0) ? textures_.deadRight : textures_.deadLeft;
		break;
	}
	}

	if (sprite_) {
		sprite_->SetTextureHandle(activeTexture);
		float srcX = static_cast<float>(currentFrame_) * kFrameWidth;
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

	UpdateAnimation();
}

void Player::Draw(const Vector2& camera) {
	if (!sprite_) {
		return;
	}

	if (invincibleTimer_ > 0 && (invincibleTimer_ / 2) % 2 == 0) {
		sprite_->SetColor({0.4f, 0.8f, 1.0f, 0.4f});
	} else if (dashTimer_ > 0) {
		sprite_->SetColor({1.0f, 1.0f, 0.4f, 1.0f});
	} else {
		sprite_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	}

	sprite_->SetPosition({position_.x - camera.x, position_.y - camera.y});
	sprite_->SetSize(size_);
	sprite_->Draw();
}