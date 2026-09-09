#include "Boss.h"
#include "MapChipField.h"
#include "Player.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

Boss::~Boss() {
	delete sprite_;
	delete buttonSprite_;
	for (int i = 0; i < 8; ++i) {
		delete debugLines_[i];
	}
}

void Boss::Initialize(BossID id, const BossTextureSet& textures, const Vector2& position, uint32_t buttonTexture) {
	id_ = id;
	textures_ = textures;
	position_ = position;
	buttonTexture_ = buttonTexture;

	// フェーズ初期設定
	if (id_ == BossID::kCombined) {
		phase_ = Phase::kPhase2_Combined;
		maxHp_ = 20; // 合体時は強化HP
	} else {
		phase_ = Phase::kPhase1_TwoBosses;
		maxHp_ = 10; // 単体時HP
	}
	hp_ = maxHp_;

	size_ = {200.0f, 300.0f}; // 200x300 固定
	velocity_ = {0.0f, 0.0f};
	state_ = State::kIdle;
	stateTimer_ = 0;
	attackCooldown_ = 60;
	hitFlash_ = 0;
	isAttacking_ = false;

	animTimer_ = 0;
	currentFrame_ = 0;
	maxFrames_ = 4;
	isFacingLeft_ = true;

	// ボス用スプライト生成
	if (!sprite_) {
		sprite_ = Sprite::Create(textures_.walk.left, {0.0f, 0.0f});
	} else {
		sprite_->SetTextureHandle(textures_.walk.left);
	}
	sprite_->SetSize(size_);

	// 落下ボタン用スプライト生成 (40x40)
	if (buttonTexture_ != 0) {
		if (!buttonSprite_) {
			buttonSprite_ = Sprite::Create(buttonTexture_, {0.0f, 0.0f});
		} else {
			buttonSprite_->SetTextureHandle(buttonTexture_);
		}
		buttonSprite_->SetSize({40.0f, 40.0f});
	}

	buttons_.clear();

	// デバッグ枠線用スプライトの初期化 (8本分: 0~3は緑枠、4~7は赤枠)
	uint32_t lineTex = (buttonTexture_ != 0) ? buttonTexture_ : textures_.walk.left;
	for (int i = 0; i < 8; ++i) {
		if (!debugLines_[i]) {
			debugLines_[i] = Sprite::Create(lineTex, {0.0f, 0.0f});
		}
	}
}

AABB2 Boss::GetAABB() const {
	float marginX = boxMargin_.x;
	float marginY = 0.0f;

	if (state_ == State::kPress) {
		marginY = size_.y * 0.25f * 0.5f;
	}

	return {
	    {position_.x + marginX,           position_.y + marginY          },
        {position_.x + size_.x - marginX, position_.y + size_.y - marginY}
    };
}

AABB2 Boss::GetAttackAABB() const {
	if (!isAttacking_) {
		return {
		    {0.0f, 0.0f},
            {0.0f, 0.0f}
        };
	}

	Vector2 attackMin = position_;
	Vector2 attackMax = {position_.x + size_.x, position_.y + size_.y};

	if (isFacingLeft_) {
		attackMin.x -= attackSize_.x;
	} else {
		attackMax.x += attackSize_.x;
	}

	return {attackMin, attackMax};
}

void Boss::OnCinchHit(int damage) {
	if (hp_ <= 0)
		return;
	hp_ -= damage;
	if (hp_ < 0)
		hp_ = 0;
	hitFlash_ = 10;
}

void Boss::ApplyKnockback(const Vector2& velocity) { (void)velocity; }

void Boss::CombineTo(const Vector2& combinePos, const BossTextureSet& combinedTextures) {
	id_ = BossID::kCombined;
	phase_ = Phase::kPhase2_Combined;
	textures_ = combinedTextures;
	position_ = combinePos;
	maxHp_ = 40;
	hp_ = maxHp_;
	state_ = State::kIdle;
	stateTimer_ = 0;
}

void Boss::Update(MapChipField* mapChipField, Player* player) {
	UpdateStitchCoolDown();
	if (hp_ <= 0)
		return;

	if (hitFlash_ > 0)
		--hitFlash_;

	// ★ 1. 落下ボタン（40x40）の更新処理
	for (auto& btn : buttons_) {
		if (!btn.isAlive)
			continue;

		btn.velocity.y += 0.45f;
		btn.position.y += btn.velocity.y;

		if (mapChipField) {
			AABB2 bAABB = btn.GetAABB();
			float resolvedY = btn.position.y;
			bool landed = false;
			if (mapChipField->ResolveBlockY(bAABB, resolvedY, btn.velocity.y, landed)) {
				if (landed)
					btn.isAlive = false;
			}
		}

		if (btn.position.y > 1000.0f)
			btn.isAlive = false;
	}

	buttons_.erase(std::remove_if(buttons_.begin(), buttons_.end(), [](const FallingButton& b) { return !b.isAlive; }), buttons_.end());

	// ★ 2. AI思考処理
	ProcessAI(player);

	// ★ 3. 移動 & 物理演算
	position_.x += velocity_.x;
	position_.y += velocity_.y;

	if (state_ == State::kPress) {
		velocity_.y += 0.55f;

		if (player) {
			const float kAirMoveSpeed = 1.5f;
			Vector2 pCenter = player->GetCenter();
			Vector2 myCenter = {position_.x + size_.x * 0.5f, position_.y + size_.y * 0.5f};

			if (pCenter.x > myCenter.x + 10.0f) {
				position_.x += kAirMoveSpeed;
			} else if (pCenter.x < myCenter.x - 10.0f) {
				position_.x -= kAirMoveSpeed;
			}
		}
	}

	// マップ衝突判定
	if (mapChipField) {
		// --- X軸の衝突判定 ---
		AABB2 aabb = GetAABB();
		float resolvedX = aabb.min.x;

		float moveDirX = 0.0f;
		if (velocity_.x > 0.0f)
			moveDirX = 1.0f;
		else if (velocity_.x < 0.0f)
			moveDirX = -1.0f;

		if (moveDirX != 0.0f && mapChipField->ResolveBlockX(aabb, resolvedX, moveDirX)) {
			position_.x = resolvedX - boxMargin_.x;
			velocity_.x = 0.0f;

			if (state_ == State::kCharge) {
				state_ = State::kIdle;
				stateTimer_ = 40;
				isAttacking_ = false;
			}
		}

		// --- Y軸の衝突判定 ---
		aabb = GetAABB();
		float resolvedY = aabb.min.y;
		bool landed = false;

		// ★ 第5引数に true を渡し、ボスは空中ブロック（一方向ブロック）をすり抜けて最下層のみ着地する
		if (mapChipField->ResolveBlockY(aabb, resolvedY, velocity_.y, landed, true)) {
			float marginY = (state_ == State::kPress) ? (size_.y * 0.25f * 0.5f) : 0.0f;
			position_.y = resolvedY - marginY;

			if (landed) {
				velocity_.y = 0.0f;
				if (state_ == State::kPress) {
					state_ = State::kIdle;
					stateTimer_ = 50;
					velocity_.x = 0.0f;
					isAttacking_ = false;
				}
			}
		}
	}

	if (position_.y < 0.0f) {
		position_.y = 0.0f;
		if (velocity_.y < 0.0f) {
			velocity_.y = 0.0f;
		}
	}

	UpdateAnimation();
}

void Boss::ProcessAI(Player* player) {
	if (attackCooldown_ > 0)
		--attackCooldown_;

	Vector2 pCenter = player ? player->GetCenter() : Vector2{0.0f, 0.0f};
	Vector2 myCenter = {position_.x + size_.x * 0.5f, position_.y + size_.y * 0.5f};
	float distToPlayer = std::abs(pCenter.x - myCenter.x);

	if (!isAttacking_ && player) {
		isFacingLeft_ = (pCenter.x < myCenter.x);
	}

	switch (state_) {
	case State::kIdle:
		velocity_.x = 0.0f;
		if (stateTimer_ > 0) {
			--stateTimer_;
			break;
		}

		if (attackCooldown_ <= 0 && player) {
			int randVal = rand() % 100;

			if (phase_ == Phase::kPhase1_TwoBosses) {
				if (distToPlayer < 280.0f) {
					state_ = State::kPunch;
					stateTimer_ = 32;
					isAttacking_ = true;
					attackSize_ = {120.0f, 150.0f};
				} else if (distToPlayer >= 500.0f) {
					state_ = State::kCharge;
					stateTimer_ = 80;
					velocity_.x = isFacingLeft_ ? -7.0f : 7.0f;
					isAttacking_ = true;
					attackSize_ = {40.0f, 0.0f};
				} else if (randVal < 50) {
					state_ = State::kCharge;
					stateTimer_ = 80;
					velocity_.x = isFacingLeft_ ? -7.0f : 7.0f;
					isAttacking_ = true;
					attackSize_ = {40.0f, 0.0f};
				} else {
					state_ = State::kPress;
					velocity_.y = -11.0f;
					velocity_.x = 0.0f;
					isAttacking_ = true;
				}
				attackCooldown_ = 180;

			} else {
				if (distToPlayer < 280.0f) {
					if (randVal < 70) {
						state_ = State::kPunch;
						stateTimer_ = 28;
						isAttacking_ = true;
						attackSize_ = {180.0f, 200.0f};
					} else {
						state_ = State::kPress;
						velocity_.y = -13.0f;
						velocity_.x = 0.0f;
						isAttacking_ = true;
					}
				} else {
					if (randVal < 40) {
						state_ = State::kCharge;
						stateTimer_ = 90;
						velocity_.x = isFacingLeft_ ? -10.0f : 10.0f;
						isAttacking_ = true;
						attackSize_ = {60.0f, 0.0f};
					} else if (randVal < 70) {
						state_ = State::kPress;
						velocity_.y = -13.0f;
						velocity_.x = 0.0f;
						isAttacking_ = true;
					} else {
						state_ = State::kRainButtons;
						stateTimer_ = 60;
					}
				}
				attackCooldown_ = 90;
			}

		} else {
			state_ = State::kWalk;
			stateTimer_ = 0;
		}
		break;

	case State::kWalk:
		if (player) {
			float speed = (phase_ == Phase::kPhase2_Combined) ? 2.2f : 1.4f;
			velocity_.x = isFacingLeft_ ? -speed : speed;
			stateTimer_++;
		}

		if ((distToPlayer < 280.0f || stateTimer_ > 150) && attackCooldown_ <= 0) {
			state_ = State::kIdle;
			stateTimer_ = 15;
		}
		break;

	case State::kPunch:
		if (stateTimer_ > 0) {
			--stateTimer_;
			if (stateTimer_ > 12) {
				const float kPunchStepSpeed = 3.5f;
				velocity_.x = isFacingLeft_ ? -kPunchStepSpeed : kPunchStepSpeed;
			} else {
				velocity_.x = 0.0f;
			}
		} else {
			state_ = State::kIdle;
			stateTimer_ = 45;
			isAttacking_ = false;
			velocity_.x = 0.0f;
		}
		break;

	case State::kCharge:
		if (stateTimer_ > 0) {
			--stateTimer_;
		} else {
			state_ = State::kIdle;
			stateTimer_ = 45;
			isAttacking_ = false;
			velocity_.x = 0.0f;
		}
		break;

	case State::kRainButtons:
		velocity_.x = 0.0f;
		if (stateTimer_ % 12 == 0 && player) {
			FallingButton btn;
			float spawnX = pCenter.x + static_cast<float>((rand() % 360) - 180);
			btn.position = {spawnX, pCenter.y - 250.0f};
			btn.velocity = {0.0f, 2.0f};
			btn.size = {40.0f, 40.0f};
			btn.isAlive = true;
			buttons_.push_back(btn);
		}

		if (stateTimer_ > 0) {
			--stateTimer_;
		} else {
			state_ = State::kIdle;
			stateTimer_ = 60;
		}
		break;

	case State::kPress:
		// 着地判定はUpdate()側のマップ衝突処理(ResolveBlockYのlanded)に任せる。
		// ここで速度ベースの疑似判定をすると、上昇→下降の切り替わり地点(頂点)で
		// velocity_.yがほぼ0になり「着地した」と誤検出して空中で静止するバグになるため、
		// このcaseでは状態遷移を行わない。
		break;
	}
}

void Boss::UpdateAnimation() {
	animTimer_++;
	currentFrame_ = (animTimer_ / kFrameInterval) % maxFrames_;
}

void Boss::DrawDebugFrame(const Vector2& camera) {
	if (hp_ <= 0 || !debugLines_[0])
		return;

	const float kLineThickness = 2.0f;

	auto drawRectLines = [&](const Vector2& pos, const Vector2& sz, const Vector4& color, int offset) {
		Vector2 screenPos = {pos.x - camera.x, pos.y - camera.y};

		debugLines_[offset + 0]->SetPosition(screenPos);
		debugLines_[offset + 0]->SetSize({sz.x, kLineThickness});
		debugLines_[offset + 0]->SetColor(color);
		debugLines_[offset + 0]->Draw();

		debugLines_[offset + 1]->SetPosition({screenPos.x, screenPos.y + sz.y - kLineThickness});
		debugLines_[offset + 1]->SetSize({sz.x, kLineThickness});
		debugLines_[offset + 1]->SetColor(color);
		debugLines_[offset + 1]->Draw();

		debugLines_[offset + 2]->SetPosition(screenPos);
		debugLines_[offset + 2]->SetSize({kLineThickness, sz.y});
		debugLines_[offset + 2]->SetColor(color);
		debugLines_[offset + 2]->Draw();

		debugLines_[offset + 3]->SetPosition({screenPos.x + sz.x - kLineThickness, screenPos.y});
		debugLines_[offset + 3]->SetSize({kLineThickness, sz.y});
		debugLines_[offset + 3]->SetColor(color);
		debugLines_[offset + 3]->Draw();
	};

	drawRectLines(position_, size_, {0.0f, 1.0f, 0.0f, 1.0f}, 0);

	AABB2 aabb = GetAABB();
	Vector2 aabbPos = aabb.min;
	Vector2 aabbSize = {aabb.max.x - aabb.min.x, aabb.max.y - aabb.min.y};
	drawRectLines(aabbPos, aabbSize, {1.0f, 0.0f, 0.0f, 1.0f}, 4);
}

void Boss::Draw(const Vector2& camera) {
	if (!sprite_ || hp_ <= 0)
		return;

	uint32_t handle = 0;
	maxFrames_ = 4;

	switch (state_) {
	case State::kWalk:
	case State::kIdle:
	case State::kCharge:
		handle = isFacingLeft_ ? textures_.walk.left : textures_.walk.right;
		maxFrames_ = 4;
		break;

	case State::kPunch:
	case State::kRainButtons:
		handle = isFacingLeft_ ? textures_.punch.left : textures_.punch.right;
		maxFrames_ = 4;
		break;

	case State::kPress:
		handle = textures_.press;
		maxFrames_ = 8;
		break;
	}

	sprite_->SetTextureHandle(handle);

	float uLeft = static_cast<float>(currentFrame_ % maxFrames_) * kFrameWidth;
	sprite_->SetTextureRect({uLeft, 0.0f}, {kFrameWidth, kFrameHeight});

	Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	if (hitFlash_ > 0) {
		color = {1.0f, 0.3f, 0.3f, 1.0f};
	}

	sprite_->SetColor(color);
	sprite_->SetPosition({position_.x - camera.x, position_.y - camera.y});
	sprite_->SetSize(size_);
	sprite_->Draw();

	if (buttonSprite_) {
		for (const auto& btn : buttons_) {
			if (!btn.isAlive)
				continue;
			buttonSprite_->SetPosition({btn.position.x - camera.x, btn.position.y - camera.y});
			buttonSprite_->Draw();
		}
	}
}