#include "Enemy.h"
#include "MapChipField.h"
#include "Player.h"
#include "SoundManager.h"
#include <cmath>

using namespace KamataEngine;

void Enemy::Initialize(const EnemyTextureHandles& textures, const Vector2& position, float minX, float maxX) {
	textures_ = textures;
	position_ = position;
	hp_ = kMaxHp;
	velocity_ = {};
	patrolDir_ = 1;
	state_ = State::kPatrol;
	fleeTimer_ = 0;
	isCinching_ = false;

	animTimer_ = 0;
	currentFrame_ = 0;
	isFacingLeft_ = true;

	// 範囲指定がない場合は 1280px 基準で自動設定
	if (minX == 0.0f && maxX == 0.0f) {
		ResetPatrolRangeToScreen();
	} else {
		minX_ = minX;
		maxX_ = maxX;
	}

	// スプライトが未作成なら生成、作成済みならテクスチャのみ更新
	if (!sprite_) {
		sprite_ = Sprite::Create(textures_.left, {0.0f, 0.0f});
	} else {
		sprite_->SetTextureHandle(textures_.left);
	}
	sprite_->SetSize(size_);
}

void Enemy::ResetPatrolRangeToScreen() {
	const float kTileWidth = 64.0f;
	const float enemyWidth = GetSize().x;

	// ヘッダーで定義した kScreenWidth をそのまま使用
	minX_ = 100.0f + (kTileWidth * 2.0f);                               // 228.0f
	maxX_ = (kScreenWidth - 100.0f - enemyWidth) - (kTileWidth * 2.0f); // 1004.0f
}

AABB2 Enemy::GetAABB() const {
	AABB2 aabb;
	aabb.min = position_;
	aabb.max = {position_.x + size_.x, position_.y + size_.y};
	return aabb;
}

void Enemy::OnCinchHit(int damage) {
	if (hp_ <= 0) {
		return;
	}
	hp_ -= damage;
	SoundManager::GetInstance()->PlaySE("Hit", 0.3f);
	if (hp_ < 0) {
		hp_ = 0;
	}
	hitFlash_ = 8;

	// ダメージを受けてHPが1になった「その瞬間」だけ逃走フラグと5秒タイマーを立てる
	if (hp_ == 1 && state_ != State::kFlee) {
		state_ = State::kFlee;
		fleeTimer_ = kFleeDuration;
	}
}

void Enemy::ApplyKnockback(const Vector2& velocity) { velocity_ = velocity; }

void Enemy::Update(MapChipField* mapChipField, Player* player) {
	UpdateStitchCoolDown();
	if (hp_ <= 0) {
		return;
	}
	if (hitFlash_ > 0) {
		--hitFlash_;
	}

	// 逃走開始時（1フレーム目）にプレイヤーの反対方向を決定する
	if (state_ == State::kFlee && fleeTimer_ == kFleeDuration) {
		if (player) {
			float playerX = player->GetCenter().x;
			float enemyX = position_.x + size_.x * 0.5f;
			patrolDir_ = (enemyX >= playerX) ? 1 : -1;
		}
	}

	// 逃走タイマーのカウントダウンと通常復帰
	if (state_ == State::kFlee) {
		--fleeTimer_;
		if (fleeTimer_ <= 0) {
			state_ = State::kPatrol;    // 5秒後に通常巡回へ復帰
			ResetPatrolRangeToScreen(); // 巡回エリアを画面幅基準にリセット
		}
	}

	// ★ 手繰り寄せ中（isCinching_）は独自の歩行移動を停止する（HookStitchのスライド移動に任せる）
	if (isCinching_) {
		velocity_.x = 0.0f;
	} else if (std::abs(velocity_.x) < 1.0f) {
		if (state_ == State::kPatrol) {
			// 【通常時】
			if (position_.x < minX_) {
				patrolDir_ = 1;
			} else if (position_.x > maxX_) {
				patrolDir_ = -1;
			}

			position_.x += static_cast<float>(patrolDir_) * kPatrolSpeed;
		} else if (state_ == State::kFlee) {
			// 【ピンチ時】
			position_.x += static_cast<float>(patrolDir_) * kFleeSpeed;
		}
		velocity_.x *= 0.8f;
	} else {
		// 吹き飛び移動中
		position_.x += velocity_.x;
		velocity_.x *= 0.90f;
	}

	// ブロック（壁）との衝突判定
	if (mapChipField) {
		AABB2 aabb = GetAABB();
		float resolvedX = position_.x;
		if (mapChipField->ResolveBlockX(aabb, resolvedX, static_cast<float>(patrolDir_))) {
			position_.x = resolvedX;
			patrolDir_ *= -1; // 壁にぶつかったら反転
		}
	}

	// 重力・縦判定（手繰り寄せ中も重力と床判定を生かすことで地面にピタッと接地して滑る）
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

	// ★ 進行方向（patrolDir_）に基づいて左右の向きを設定
	if (patrolDir_ < 0) {
		isFacingLeft_ = true;
	} else if (patrolDir_ > 0) {
		isFacingLeft_ = false;
	}

	// ★ アニメーションタイマーとコマ番号の更新
	animTimer_++;
	currentFrame_ = (animTimer_ / kFrameInterval) % kNumFrames;
}

void Enemy::Draw(const Vector2& camera) {
	if (!sprite_ || hp_ <= 0) {
		return;
	}

	// ★ 向きに応じてテクスチャを切り替え
	uint32_t handle = isFacingLeft_ ? textures_.left : textures_.right;
	sprite_->SetTextureHandle(handle);

	// ★ 横4コマのアニメーション切り抜き（UV）を設定
	float uLeft = static_cast<float>(currentFrame_) * kFrameWidth;
	sprite_->SetTextureRect({uLeft, 0.0f}, {kFrameWidth, kFrameHeight});

	// 通常時は白、逃走中は黄色に変化、被弾時は赤み
	Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	if (state_ == State::kFlee) {
		color = {0.95f, 0.8f, 0.2f, 1.0f};
	}
	if (hitFlash_ > 0) {
		color = {1.0f, 0.3f, 0.3f, 1.0f};
	}

	sprite_->SetColor(color);
	sprite_->SetPosition({position_.x - camera.x, position_.y - camera.y});
	sprite_->SetSize(size_);
	sprite_->Draw();
}

void Enemy::CheckEnemyCollisions(std::vector<Enemy*>& enemies) {
	for (size_t i = 0; i < enemies.size(); ++i) {
		for (size_t j = i + 1; j < enemies.size(); ++j) {
			Enemy* a = enemies[i];
			Enemy* b = enemies[j];

			if (!a || !b || a->IsDead() || b->IsDead()) {
				continue;
			}

			AABB2 boxA = a->GetAABB();
			AABB2 boxB = b->GetAABB();

			// AABB同士の当たり判定チェック
			if (IsCollision(boxA, boxB)) {
				// X軸の重なり量を計算
				float overlapX = 0.0f;
				if (a->GetPosition().x < b->GetPosition().x) {
					overlapX = boxA.max.x - boxB.min.x;
				} else {
					overlapX = boxB.max.x - boxA.min.x;
				}

				if (overlapX > 0.0f) {
					// 半分ずつお互いを押し返す
					float push = overlapX * 0.5f;
					Vector2 posA = a->GetPosition();
					Vector2 posB = b->GetPosition();

					if (posA.x < posB.x) {
						posA.x -= push;
						posB.x += push;
						a->patrolDir_ = -1;
						b->patrolDir_ = 1;
					} else {
						posA.x += push;
						posB.x -= push;
						a->patrolDir_ = 1;
						b->patrolDir_ = -1;
					}

					a->SetPosition(posA);
					b->SetPosition(posB);
				}
			}
		}
	}
}