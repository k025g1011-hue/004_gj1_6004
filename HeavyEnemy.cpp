#define NOMINMAX
#include "HeavyEnemy.h"
#include "MapChipField.h"
#include "Player.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void HeavyEnemy::Initialize(uint32_t textureHandle, const Vector2& position) {
	sprite_ = Sprite::Create(textureHandle, {0.0f, 0.0f});
	position_ = position;
	hp_ = kMaxHp;
	velocity_ = {};
	state_ = State::kPatrol;
	hitFlash_ = 0;
	dir_ = 1;

	ResetPatrolRangeToScreen();

	warningTimer_ = 0;
	jumpCount_ = 0;
	cooldownTimer_ = 0;
	chargeTimer_ = 0;
	isBraking_ = false;
}

void HeavyEnemy::ResetPatrolRangeToScreen() {
	const float kTileWidth = 64.0f;
	const float enemyWidth = size_.x;

	patrolLeft_ = kMarginX + (kTileWidth * 2.0f);
	patrolRight_ = (kScreenWidth - kMarginX - enemyWidth) - (kTileWidth * 2.0f);
}

AABB2 HeavyEnemy::GetAABB() const {
	AABB2 aabb;
	aabb.min = position_;
	aabb.max = {position_.x + size_.x, position_.y + size_.y};
	return aabb;
}

void HeavyEnemy::OnCinchHit(int damage) {
	if (hp_ <= 0) {
		return;
	}
	hp_ -= damage;
	if (hp_ < 0) {
		hp_ = 0;
	}
	hitFlash_ = 8;
}

void HeavyEnemy::ApplyKnockback(const Vector2& velocity) { (void)velocity; }

void HeavyEnemy::Update(MapChipField* mapChipField, Player* player) {
	UpdateStitchCoolDown();
	if (hp_ <= 0) {
		return;
	}
	if (hitFlash_ > 0) {
		--hitFlash_;
	}

	const bool isPinch = (hp_ <= 2);

	Vector2 pCenter = player ? player->GetCenter() : Vector2{0, 0};
	Vector2 myCenter = {position_.x + size_.x * 0.5f, position_.y + size_.y * 0.5f};
	float distToPlayer = std::abs(pCenter.x - myCenter.x);

	switch (state_) {
	case State::kPatrol: {
		float speed = isPinch ? 2.2f : 1.2f;
		position_.x += static_cast<float>(dir_) * speed;

		if (position_.x <= patrolLeft_ && dir_ < 0) {
			dir_ = 1;
		} else if (position_.x >= patrolRight_ && dir_ > 0) {
			dir_ = -1;
		}

		if (mapChipField) {
			AABB2 aabb = GetAABB();
			float resolvedX = position_.x;
			if (mapChipField->ResolveBlockX(aabb, resolvedX, static_cast<float>(dir_))) {
				position_.x = resolvedX;
				dir_ *= -1;
			}
		}

		if (player && !player->IsDead() && distToPlayer < 320.0f) {
			dir_ = (pCenter.x > myCenter.x) ? 1 : -1;
			state_ = State::kWarning;
			warningTimer_ = 0;
			jumpCount_ = 0;
			velocity_.x = 0.0f;
		}
		break;
	}

	case State::kWarning: {
		velocity_.x = 0.0f;
		++warningTimer_;

		if (warningTimer_ % 12 == 1 && jumpCount_ < 2) {
			velocity_.y = -5.0f;
			++jumpCount_;
		}

		if (jumpCount_ >= 2 && warningTimer_ > 30) {
			state_ = State::kCharge;
			velocity_.x = static_cast<float>(dir_) * (isPinch ? 11.0f : 7.0f);
			chargeTimer_ = 240; // 最大4秒間突撃
			isBraking_ = false;
		}
		break;
	}

	case State::kCharge: {
		// ★ プレイヤーが目の前から消えた（背後に回った/死んだ）か判定
		if (!isBraking_) {
			bool playerBehind = false;
			if (!player || player->IsDead()) {
				playerBehind = true;
			} else {
				// 右に突撃中なのにプレイヤーが左にいる、または左に突撃中なのにプレイヤーが右にいる
				if ((dir_ > 0 && pCenter.x < myCenter.x) || (dir_ < 0 && pCenter.x > myCenter.x)) {
					playerBehind = true;
				}
			}

			// 目の前から消えたらブレーキ開始
			if (playerBehind) {
				isBraking_ = true;
			}
		}

		// ブレーキ中の処理（徐々に減速）
		if (isBraking_) {
			velocity_.x *= 0.98f; // 毎フレーム減速（ズザーッと滑る表現）
		}

		position_.x += velocity_.x;
		--chargeTimer_;

		bool hitWall = false;

		// 壁判定
		if (mapChipField) {
			AABB2 aabb = GetAABB();
			float resolvedX = position_.x;
			if (mapChipField->ResolveBlockX(aabb, resolvedX, static_cast<float>(dir_))) {
				position_.x = resolvedX;
				hitWall = true;
			}
		}

		// 完全に停止した（|velocity.x| < 0.5）、時間切れ、または壁衝突で隙へ
		if (std::abs(velocity_.x) < 0.5f || chargeTimer_ <= 0 || hitWall) {
			state_ = State::kCooldown;
			cooldownTimer_ = 120; // 隙の時間
			velocity_.x = 0.0f;
			isBraking_ = false;

			if (hitWall) {
				dir_ *= -1;
			}
		}
		break;
	}

	case State::kCooldown: {
		velocity_.x = 0.0f;
		--cooldownTimer_;

		if (cooldownTimer_ <= 0) {
			state_ = State::kPatrol;
			warningTimer_ = 0;
			jumpCount_ = 0;

			ResetPatrolRangeToScreen();
		}
		break;
	}
	}

	// 重力および Y軸判定
	velocity_.y += kGravity;
	position_.y += velocity_.y;

	if (mapChipField) {
		AABB2 aabb = GetAABB();
		float resolvedY = position_.y;
		bool landed = false;
		if (mapChipField->ResolveBlockY(aabb, resolvedY, velocity_.y, landed)) {
			position_.y = resolvedY;
			if (landed && velocity_.y > 0.0f) {
				velocity_.y = 0.0f;
			}
		}
	}
}

void HeavyEnemy::Draw(const Vector2& camera) {
	if (!sprite_ || hp_ <= 0) {
		return;
	}

	Vector4 color = (hp_ <= 2) ? Vector4{0.9f, 0.35f, 0.35f, 1.0f} : Vector4{0.45f, 0.5f, 0.6f, 1.0f};
	if (hitFlash_ > 0) {
		color = {1.0f, 1.0f, 1.0f, 1.0f};
	}

	sprite_->SetColor(color);
	sprite_->SetPosition({position_.x - camera.x, position_.y - camera.y});
	sprite_->SetSize(size_);
	sprite_->Draw();
}