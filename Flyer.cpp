#define NOMINMAX
#include "Flyer.h"
#include "MapChipField.h"
#include "Player.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void Flyer::Initialize(uint32_t textureHandle, const Vector2& position, float minX, float maxX) {
	sprite_ = Sprite::Create(textureHandle, {0.0f, 0.0f});
	position_ = position;
	basePatrolPos_ = position;
	spawnY_ = position.y; // 初期スポーン時の高度を記憶
	hp_ = kMaxHp;
	velocity_ = {};
	direction_ = 1;
	state_ = State::kPatrol;
	sinAngle_ = 0.0f;
	attackTimer_ = 0;

	// 180〜360フレーム（3秒〜6秒）の範囲で最初のランダム間隔を決定
	std::uniform_int_distribution<int> dist(180, 360);
	nextAttackInterval_ = dist(randomEngine_);

	if (minX == 0.0f && maxX == 0.0f) {
		ResetPatrolRangeToSection();
	} else {
		minX_ = minX;
		maxX_ = maxX;
	}
}

void Flyer::ResetPatrolRangeToSection() {
	const float kTileWidth = 64.0f;
	minX_ = 100.0f + (kTileWidth * 2.0f);
	maxX_ = (kScreenWidth - 100.0f - size_.x) - (kTileWidth * 2.0f);
}

AABB2 Flyer::GetAABB() const {
	AABB2 aabb;
	aabb.min = position_;
	aabb.max = {position_.x + size_.x, position_.y + size_.y};
	return aabb;
}

void Flyer::OnCinchHit(int damage) {
	if (hp_ <= 0) {
		return;
	}
	hp_ -= damage;
	if (hp_ < 0) {
		hp_ = 0;
	}
	hitFlash_ = 8;
	// ※逃走はせず、HPが減ることで狂暴化（スピード・攻撃頻度アップ）する仕様
}

void Flyer::ApplyKnockback(const Vector2& velocity) { velocity_ = velocity; }

void Flyer::Update(MapChipField* mapChipField, Player* player) {
	(void)mapChipField;
	UpdateStitchCoolDown();
	if (hp_ <= 0) {
		return;
	}
	if (hitFlash_ > 0) {
		--hitFlash_;
	}

	// HPが減っている場合は狂暴化
	float speedMult = (hp_ < kMaxHp) ? 1.5f : 1.0f;
	float attackIntervalMult = (hp_ < kMaxHp) ? 0.7f : 1.0f;

	// 吹き飛び・引っ張られ移動中の処理（重力なし・減衰あり）
	if (std::abs(velocity_.x) > 1.0f || std::abs(velocity_.y) > 1.0f) {
		position_.x += velocity_.x;
		position_.y += velocity_.y;
		velocity_.x *= 0.90f;
		velocity_.y *= 0.90f;
		basePatrolPos_.x = position_.x;
		return;
	}

	switch (state_) {
	case State::kPatrol:
		UpdatePatrol(player, speedMult, attackIntervalMult);
		break;
	case State::kCharge:
		UpdateCharge(player, speedMult);
		break;
	case State::kReturn:
		UpdateReturn(speedMult);
		break;
	}
}

void Flyer::UpdatePatrol(Player* player, float speedMult, float attackIntervalMult) {
	// 1. 横方向の巡回
	float patrolSpeed = 2.0f * speedMult;
	basePatrolPos_.x += patrolSpeed * static_cast<float>(direction_);

	if (basePatrolPos_.x >= maxX_) {
		basePatrolPos_.x = maxX_;
		direction_ = -1;
	} else if (basePatrolPos_.x <= minX_) {
		basePatrolPos_.x = minX_;
		direction_ = 1;
	}

	// 2. 上下のSine波フワフワ移動
	sinAngle_ += 0.05f * speedMult;
	if (sinAngle_ >= 6.2831853f) {
		sinAngle_ -= 6.2831853f;
	}
	float waveOffset = std::sin(sinAngle_) * 20.0f;

	position_.x = basePatrolPos_.x;
	position_.y = basePatrolPos_.y + waveOffset;

	// 3. 突撃タイマー（HP減少で間隔が短縮される）
	++attackTimer_;
	// ランダムに決まった目標間隔に倍率をかける
	int targetInterval = static_cast<int>(static_cast<float>(nextAttackInterval_) * attackIntervalMult);

	if (attackTimer_ >= targetInterval && player) {
		attackTimer_ = 0;
		// 次回の突撃間隔をランダムに再設定 (180〜360フレーム)
		std::uniform_int_distribution<int> dist(180, 360);
		nextAttackInterval_ = dist(randomEngine_);
		state_ = State::kCharge;
		chargeStartPos_ = position_;
		chargeTargetPos_ = player->GetCenter();
		chargeProgress_ = 0.0f;
	}
}

void Flyer::UpdateCharge(Player* player, float speedMult) {
	(void)player;
	chargeProgress_ += 0.010f * speedMult;

	if (chargeProgress_ >= 1.0f) {
		chargeProgress_ = 1.0f;
		state_ = State::kReturn;
		return;
	}

	float t = chargeProgress_;
	// 二次ベジェ曲線（円弧を描いて突撃）
	Vector2 controlPoint = {(chargeStartPos_.x + chargeTargetPos_.x) * 0.5f, std::max(chargeStartPos_.y, chargeTargetPos_.y) + 60.0f};

	float u = 1.0f - t;
	position_.x = u * u * chargeStartPos_.x + 2.0f * u * t * controlPoint.x + t * t * chargeTargetPos_.x;
	position_.y = u * u * chargeStartPos_.y + 2.0f * u * t * controlPoint.y + t * t * chargeTargetPos_.y;
}

void Flyer::UpdateReturn(float speedMult) {
	basePatrolPos_.y += (spawnY_ - basePatrolPos_.y) * 0.1f;

	Vector2 targetPos = {basePatrolPos_.x, basePatrolPos_.y + std::sin(sinAngle_) * 20.0f};
	Vector2 diff = {targetPos.x - position_.x, targetPos.y - position_.y};
	float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

	float returnSpeed = 4.0f * speedMult;
	if (dist <= returnSpeed) {
		position_ = targetPos;
		basePatrolPos_.y = spawnY_; // 完全に元の高度にリセット
		state_ = State::kPatrol;
	} else {
		position_.x += (diff.x / dist) * returnSpeed;
		position_.y += (diff.y / dist) * returnSpeed;
	}
}

void Flyer::Draw(const Vector2& camera) {
	if (!sprite_ || hp_ <= 0) {
		return;
	}

	// 飛行敵は空色/紫系。HPが減ると赤みがかる（狂暴化表現）
	Vector4 color = {0.4f, 0.8f, 0.95f, 1.0f};
	if (hp_ < kMaxHp) {
		color = {0.95f, 0.4f, 0.4f, 1.0f};
	}
	if (hitFlash_ > 0) {
		color = {1.0f, 1.0f, 1.0f, 1.0f};
	}

	sprite_->SetColor(color);
	sprite_->SetPosition({position_.x - camera.x, position_.y - camera.y});
	sprite_->SetSize(size_);
	sprite_->Draw();
}