#include "RangedEnemy.h"
#include "MapChipField.h"
#include "Player.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void RangedEnemy::Initialize(const EnemyTextureHandles& textures, const EnemyBulletTextureHandles& bulletTextures, const Vector2& position) {
	textures_ = textures;
	bulletTextures_ = bulletTextures;

	if (!bulletSpriteLeft_) {
		bulletSpriteLeft_ = Sprite::Create(bulletTextures_.left, {0.0f, 0.0f});
	} else {
		bulletSpriteLeft_->SetTextureHandle(bulletTextures_.left);
	}

	if (!bulletSpriteRight_) {
		bulletSpriteRight_ = Sprite::Create(bulletTextures_.right, {0.0f, 0.0f});
	} else {
		bulletSpriteRight_->SetTextureHandle(bulletTextures_.right);
	}

	position_ = position;
	hp_ = kMaxHp;
	velocity_ = {};
	moveDir_ = 1;
	shotTimer_ = kNormalShotInterval;
	bullets_.clear();
	isCinching_ = false;

	animTimer_ = 0;
	currentFrame_ = 0;
	isFacingLeft_ = true;

	if (!sprite_) {
		sprite_ = Sprite::Create(textures_.left, {0.0f, 0.0f});
	} else {
		sprite_->SetTextureHandle(textures_.left);
	}
	sprite_->SetSize(size_);
}

AABB2 RangedEnemy::GetAABB() const {
	return {
	    position_, {position_.x + size_.x, position_.y + size_.y}
    };
}

void RangedEnemy::OnCinchHit(int damage) {
	if (hp_ <= 0)
		return;
	hp_ -= damage;
	if (hp_ < 0)
		hp_ = 0;
	hitFlash_ = 8;
}

void RangedEnemy::ApplyKnockback(const Vector2& velocity) { velocity_ = velocity; }

void RangedEnemy::Update(MapChipField* mapChipField, Player* player) {
	UpdateStitchCoolDown();
	if (hp_ <= 0)
		return;

	if (hitFlash_ > 0)
		--hitFlash_;

	// 1. 弾の更新処理
	for (auto& bullet : bullets_) {
		if (!bullet.isAlive)
			continue;
		bullet.position.x += bullet.velocity.x;
		bullet.position.y += bullet.velocity.y;

		// 画面外またはマップ壁との衝突で削除
		if (mapChipField) {
			AABB2 bAABB = bullet.GetAABB();
			float dummyX = bullet.position.x;
			if (mapChipField->ResolveBlockX(bAABB, dummyX, (bullet.velocity.x > 0 ? 1.0f : -1.0f))) {
				bullet.isAlive = false;
			}
		}
	}
	// 死んだ弾を削除
	bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(), [](const EnemyBullet& b) { return !b.isAlive; }), bullets_.end());

	// 2. 移動思考 AI（距離維持 & 反転）
	bool isPinch = (hp_ <= kPinchHpThreshold);
	float currentSpeed = isPinch ? kPinchSpeed : kNormalSpeed;

	if (isCinching_) {
		// 手繰り寄せ中（Cinch中）は歩行入力を停止（HookStitchのスライドに委ねる）
		velocity_.x = 0.0f;
	} else if (std::abs(velocity_.x) < 1.0f) {
		if (player) {
			float enemyCenterX = position_.x + size_.x * 0.5f;
			float playerCenterX = player->GetCenter().x;
			float diffX = enemyCenterX - playerCenterX;
			float dist = std::abs(diffX);

			// プレイヤーとの距離に応じて進行方向を決定
			if (dist < kTargetDistance - kDistanceMargin) {
				// 近すぎる：逃げる（距離を取る）
				moveDir_ = (diffX > 0) ? 1 : -1;
			} else if (dist > kTargetDistance + kDistanceMargin) {
				// 遠すぎる：近づく
				moveDir_ = (diffX > 0) ? -1 : 1;
			}
			// ちょうどいい距離（誤差範囲内）の場合は現在の moveDir_ を維持
		}

		position_.x += static_cast<float>(moveDir_) * currentSpeed;
		velocity_.x *= 0.8f;
	} else {
		// 吹き飛び移動中
		position_.x += velocity_.x;
		velocity_.x *= 0.90f;
	}

	// 3. 壁衝突判定（壁に追い詰められたら反転して内側へ進む）
	if (mapChipField) {
		AABB2 aabb = GetAABB();
		float resolvedX = position_.x;
		if (mapChipField->ResolveBlockX(aabb, resolvedX, static_cast<float>(moveDir_))) {
			position_.x = resolvedX;
			moveDir_ *= -1; // ★ 壁にぶつかったら反転して内側（プレイヤー側）に向かう
		}
	}

	// 4. 重力 & 床判定（スライド接地対応）
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

	// 5. 射撃タイマー更新 & 発射
	if (!isCinching_ && player) {
		--shotTimer_;
		if (shotTimer_ <= 0) {
			Shoot(player);
			shotTimer_ = isPinch ? kPinchShotInterval : kNormalShotInterval;
		}
	}

	// ★ 本体向きの更新（プレイヤー方向優先）
	if (player) {
		isFacingLeft_ = (player->GetCenter().x < position_.x + size_.x * 0.5f);
	} else if (moveDir_ < 0) {
		isFacingLeft_ = true;
	} else if (moveDir_ > 0) {
		isFacingLeft_ = false;
	}

	// ★ 本体アニメーションタイマー更新
	animTimer_++;
	currentFrame_ = (animTimer_ / kFrameInterval) % kNumFrames;
}

void RangedEnemy::Shoot(Player* player) {
	if (!player)
		return;

	float enemyCenterX = position_.x + size_.x * 0.5f;
	float playerCenterX = player->GetCenter().x;

	// 弾の発射位置（敵の中心・Y軸は手元付近）
	Vector2 spawnPos;
	spawnPos.y = position_.y + 30.0f; // 胸の高さ

	float dirX = (playerCenterX >= enemyCenterX) ? 1.0f : -1.0f;
	if (dirX > 0) {
		spawnPos.x = position_.x + size_.x;
	} else {
		spawnPos.x = position_.x - 50.0f; // 弾サイズ横幅50px分オフセット
	}

	EnemyBullet bullet;
	bullet.position = spawnPos;
	bullet.velocity = {dirX * kBulletSpeed, 0.0f};
	bullet.isAlive = true;
	bullet.isFacingLeft = (dirX < 0.0f); // ★ 弾の飛ぶ向き（左向きなら true）

	bullets_.push_back(bullet);
}

void RangedEnemy::Draw(const Vector2& camera) {
	// ★ 弾の描画（左右専用スプライトを使い分け）
	for (const auto& bullet : bullets_) {
		if (!bullet.isAlive)
			continue;

		Sprite* targetBulletSprite = bullet.isFacingLeft ? bulletSpriteLeft_ : bulletSpriteRight_;
		if (targetBulletSprite) {
			targetBulletSprite->SetPosition({bullet.position.x - camera.x, bullet.position.y - camera.y});
			targetBulletSprite->SetSize(bullet.size);
			targetBulletSprite->Draw();
		}
	}

	if (!sprite_ || hp_ <= 0)
		return;

	// ★ 本体の描画（左右向き切り替え + 横4コマ UV 切り抜き）
	uint32_t handle = isFacingLeft_ ? textures_.left : textures_.right;
	sprite_->SetTextureHandle(handle);

	float uLeft = static_cast<float>(currentFrame_) * kFrameWidth;
	sprite_->SetTextureRect({uLeft, 0.0f}, {kFrameWidth, kFrameHeight});

	// 色設定（ピンチ時やヒットフラッシュ）
	Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	if (hp_ <= kPinchHpThreshold) {
		color = {1.0f, 0.7f, 0.7f, 1.0f};
	}
	if (hitFlash_ > 0) {
		color = {1.0f, 0.3f, 0.3f, 1.0f};
	}

	sprite_->SetColor(color);
	sprite_->SetPosition({position_.x - camera.x, position_.y - camera.y});
	sprite_->SetSize(size_);
	sprite_->Draw();
}