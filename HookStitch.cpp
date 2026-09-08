#define NOMINMAX
#include "HookStitch.h"
#include "Boss.h"
#include "Player.h"
#include <cmath>
#include <random>

using namespace KamataEngine;

void HookStitch::Initialize(uint32_t textureHandle) {
	stitched_.clear();
	for (int i = 0; i < kMaxStitch; ++i) {
		markSprites_.push_back(Sprite::Create(textureHandle, {0.0f, 0.0f}));
		markCrossSprites_.push_back(Sprite::Create(textureHandle, {0.0f, 0.0f}));
	}
	for (int i = 0; i < kDotCount; ++i) {
		dotSprites_.push_back(Sprite::Create(textureHandle, {0.0f, 0.0f}));
	}
}

bool HookStitch::Contains(StitchTarget* target) const {
	for (StitchTarget* stitched : stitched_) {
		if (stitched == target) {
			return true;
		}
	}
	return false;
}

void HookStitch::Clear() {
	stitched_.clear();
	cinchTimer_ = 0;
	pendingResolve_ = false;
}

void HookStitch::TryStitch(StitchTarget* target) {
	if (!target || !target->CanStitch() || target->IsDead()) {
		return;
	}
	if (target->IsStitchCooling()) {
		return;
	}
	if (Contains(target)) {
		return;
	}

	stitched_.push_back(target);
	target->StartStitchCoolDown(kCoolDown);

	// 古い縫いから消す
	if (static_cast<int>(stitched_.size()) > kMaxStitch) {
		stitched_.erase(stitched_.begin());
	}
}

// プレイヤーの位置も考慮して重心を計算
Vector2 HookStitch::CalcCentroid(Player* player) const {
	Vector2 sum{};
	int count = 0;

	if (player) {
		Vector2 pCenter = player->GetCenter();
		sum.x += pCenter.x;
		sum.y += pCenter.y;
		++count;
	}

	for (StitchTarget* target : stitched_) {
		if (!target) {
			continue;
		}
		Vector2 c = AABBCenter(target->GetAABB());
		sum.x += c.x;
		sum.y += c.y;
		++count;
	}

	if (count == 0) {
		return {};
	}
	return {sum.x / static_cast<float>(count), sum.y / static_cast<float>(count)};
}

// 敵同士、およびプレイヤーとの衝突判定
bool HookStitch::CheckCinchCollision(Player* player) const {
	for (size_t i = 0; i < stitched_.size(); ++i) {
		StitchTarget* a = stitched_[i];
		if (!a) {
			continue;
		}
		for (size_t j = i + 1; j < stitched_.size(); ++j) {
			StitchTarget* b = stitched_[j];
			if (!b) {
				continue;
			}
			if (a->IsFixed() && b->IsFixed()) {
				continue;
			}
			if (IsCollision(a->GetAABB(), b->GetAABB())) {
				return true;
			}
		}

		if (player && !a->IsFixed()) {
			if (IsCollision(a->GetAABB(), player->GetAABB())) {
				return true;
			}
		}
	}
	return false;
}

void HookStitch::ResolveCinch(const Vector2& centroid) {
	bool hasBossA = false;
	bool hasBossB = false;
	bool hasStake = false;
	int bossIndex = 0;
	for (StitchTarget* target : stitched_) {
		if (!target) {
			continue;
		}
		if (target->GetKind() == StitchTarget::Kind::kStake) {
			hasStake = true;
		} else if (target->GetKind() == StitchTarget::Kind::kBoss) {
			if (bossIndex == 0) {
				hasBossA = true;
			} else {
				hasBossB = true;
			}
			++bossIndex;
		}
	}

	int damage = 1;
	if (hasBossA && hasBossB && hasStake) {
		damage = 3; // 本ダメージ
	} else if (stitched_.size() >= 3) {
		damage = 2;
	}

	for (StitchTarget* target : stitched_) {
		if (!target) {
			continue;
		}
		if (target->GetKind() != StitchTarget::Kind::kBoss && target->GetKind() != StitchTarget::Kind::kFodder && target->GetKind() != StitchTarget::Kind::kFlyer) {
			continue;
		}
		target->OnCinchHit(damage);
		Vector2 center = AABBCenter(target->GetAABB());
		target->ApplyKnockback({(center.x - centroid.x) * 0.08f, -8.0f});
	}

	Clear();
}

void HookStitch::Cinch(Player* player) {
	if (cinchTimer_ > 0) {
		return;
	}
	// 1体以上縫われていれば手繰り寄せ可能
	if (stitched_.empty()) {
		return;
	}
	cinchTimer_ = kCinchDuration;
	pendingResolve_ = true;
	if (player) {
		player->SetInvincible(kCinchInvincible);
	}
}

void HookStitch::Update(Player* player, const std::vector<StitchTarget*>& targets) {
	(void)targets;
	++markAnim_;

	for (auto it = stitched_.begin(); it != stitched_.end();) {
		if (!(*it) || (*it)->IsDead()) {
			it = stitched_.erase(it);
		} else {
			++it;
		}
	}

	if (!stitched_.empty() && player) {
		float totalLength = 0.0f;
		bool shouldBreak = false;

		// 糸の経路: Player -> Target[0] -> Target[1] -> ...
		Vector2 prevPos = player->GetCenter();

		for (StitchTarget* target : stitched_) {
			if (!target) {
				continue;
			}
			Vector2 currPos = AABBCenter(target->GetAABB());

			float dx = currPos.x - prevPos.x;
			float dy = currPos.y - prevPos.y;
			float dist = std::sqrt(dx * dx + dy * dy);

			if (dist > kMaxLinkDistance) {
				shouldBreak = true;
				break;
			}

			totalLength += dist;
			prevPos = currPos;
		}

		if (totalLength > kMaxTotalLength) {
			shouldBreak = true;
		}

		// 上限を超えたら糸を即時解除する
		if (shouldBreak) {
			for (StitchTarget* target : stitched_) {
				if (target) {
					target->SetCinching(false);
				}
			}
			Clear();
			return;
		}
	}

	if (cinchTimer_ > 0) {
		const Vector2 centroid = CalcCentroid(player);
		for (StitchTarget* target : stitched_) {
			if (!target || target->IsFixed()) {
				continue;
			}

			// 手繰り寄せ中フラグを ON
			target->SetCinching(true);

			AABB2 aabb = target->GetAABB();

			Vector2 targetPoint;
			targetPoint.x = (aabb.min.x + aabb.max.x) * 0.5f;
			targetPoint.y = aabb.min.y + 24.0f;

			Vector2 diff{centroid.x - targetPoint.x, centroid.y - targetPoint.y};
			float len = std::sqrt(diff.x * diff.x + diff.y * diff.y);

			if (len > 1.0f) {
				Vector2 next = target->GetPosition();

				// X軸: 常に中心（centroid）に向かってスライド移動
				next.x += (diff.x / len) * kCinchSpeed;

				// Y軸: 中心点が敵より「上」にある場合（空中へ持ち上げる時）のみ Y 移動を適用
				// 横方向や斜め下に引っ張る時は Y を直接操作せず、敵の重力と床判定に任せてズザーッと滑らせる
				if (diff.y < -10.0f) {
					next.y += (diff.y / len) * kCinchSpeed;
				}

				target->SetPosition(next);
			}
		}

		--cinchTimer_;

		// 衝突または時間切れで決着判定へ移行
		if (pendingResolve_ && (CheckCinchCollision(player) || cinchTimer_ <= 0)) {
			for (StitchTarget* target : stitched_) {
				if (target) {
					target->SetCinching(false);
				}
			}
			ResolveCinch(centroid);
		}
		return;
	}

	Input* input = Input::GetInstance();
	if (input->TriggerKey(DIK_C) || input->TriggerKey(DIK_F)) {
		Cinch(player);
	}
}

void HookStitch::DrawThread(const Vector2& from, const Vector2& to, const Vector4& color, float dangerProgress) {
	Vector2 diff{to.x - from.x, to.y - from.y};
	float len = std::sqrt(diff.x * diff.x + diff.y * diff.y);
	if (len < 1.0f || dotSprites_.empty()) {
		return;
	}

	// 1. 間隔の伸長: 危険度が上がると 8.0f -> 最大 18.0f まで疎になる
	float spacing = 8.0f + dangerProgress * 10.0f;

	int steps = static_cast<int>(len / spacing);
	if (steps < 1) {
		steps = 1;
	}
	const int remain = static_cast<int>(dotSprites_.size()) - dotIndex_;
	if (steps > remain) {
		steps = remain;
	}

	// 単位ベクトル & 垂直（法線）ベクトルの計算（振動の方向を決める）
	Vector2 dir{diff.x / len, diff.y / len};
	Vector2 normal{-dir.y, dir.x}; // 糸に対して垂直な向き

	// 乱数生成器（ブレ演出用）
	static std::mt19937 randomEngine(1337);
	std::uniform_real_distribution<float> distOffset(-1.0f, 1.0f);

	// 2. ドットのサイズ: 伸び切ると少し細く小さくなる (6.0f -> 3.5f)
	float dotSize = 6.0f - dangerProgress * 2.5f;

	for (int i = 1; i < steps; ++i) {
		const float t = static_cast<float>(i) / static_cast<float>(steps);

		// 基本座標
		float px = from.x + diff.x * t;
		float py = from.y + diff.y * t;

		// 3. 振動ブレ: 危険度に応じたランダムオフセット（法線方向に振る）
		if (dangerProgress > 0.0f) {
			float shakeMagnitude = dangerProgress * 4.5f; // 最大4.5px揺れる
			float offset = distOffset(randomEngine) * shakeMagnitude;
			px += normal.x * offset;
			py += normal.y * offset;
		}

		Sprite* sprite = dotSprites_[dotIndex_++];
		sprite->SetRotation(0.0f);
		sprite->SetColor(color);
		sprite->SetPosition({px - dotSize * 0.5f, py - dotSize * 0.5f});
		sprite->SetSize({dotSize, dotSize});
		sprite->Draw();
	}
}

void HookStitch::DrawMark(const Vector2& center, const Vector4& color) {
	if (markSprites_.empty() || markCrossSprites_.empty()) {
		return;
	}

	static int markIndex = 0;
	if (markIndex >= static_cast<int>(markSprites_.size())) {
		markIndex = 0;
	}
	Sprite* plusH = markSprites_[markIndex];
	Sprite* plusV = markCrossSprites_[markIndex];
	++markIndex;
	if (markIndex >= static_cast<int>(markSprites_.size())) {
		markIndex = 0;
	}

	const float pulse = 1.0f + 0.10f * std::sin(static_cast<float>(markAnim_) * 0.18f);
	const float size = 22.0f * pulse;

	plusH->SetRotation(0.0f);
	plusH->SetColor(color);
	plusH->SetPosition({center.x - size * 0.5f, center.y - 3.0f});
	plusH->SetSize({size, 6.0f});
	plusH->Draw();

	plusV->SetRotation(0.0f);
	plusV->SetColor(color);
	plusV->SetPosition({center.x - 3.0f, center.y - size * 0.5f});
	plusV->SetSize({6.0f, size});
	plusV->Draw();
}

void HookStitch::Draw(Player* player, const Vector2& camera) {
	dotIndex_ = 0;

	Vector4 baseColor = (cinchTimer_ > 0) ? Vector4{1.0f, 0.25f, 0.2f, 1.0f} : Vector4{1.0f, 0.92f, 0.35f, 1.0f};
	float dangerProgress = 0.0f; // 0.0 (安全) 〜 1.0 (限界破断)

	// 距離の危険度判定（70%を超えたら進行度をアップ）
	if (cinchTimer_ <= 0 && !stitched_.empty() && player) {
		float maxRatio = 0.0f;
		float totalLength = 0.0f;
		Vector2 prevPos = player->GetCenter();

		for (StitchTarget* target : stitched_) {
			if (!target)
				continue;
			Vector2 currPos = AABBCenter(target->GetAABB());

			float dx = currPos.x - prevPos.x;
			float dy = currPos.y - prevPos.y;
			float dist = std::sqrt(dx * dx + dy * dy);

			float linkRatio = dist / kMaxLinkDistance;
			if (linkRatio > maxRatio) {
				maxRatio = linkRatio;
			}

			totalLength += dist;
			prevPos = currPos;
		}

		float totalRatio = totalLength / kMaxTotalLength;
		if (totalRatio > maxRatio) {
			maxRatio = totalRatio;
		}

		// 70%の距離を超えたら警告フェーズ
		if (maxRatio > 0.7f) {
			dangerProgress = (maxRatio - 0.7f) / 0.3f; // 0.0 〜 1.0

			// 限界に近づくにつれて激しく点滅（赤変色）
			float flashSpeed = 0.3f + dangerProgress * 0.5f; // 限界に近いほど超高速点滅
			float flash = (std::sin(static_cast<float>(markAnim_) * flashSpeed) + 1.0f) * 0.5f;

			baseColor.x = 1.0f;
			baseColor.y = (1.0f - dangerProgress * 0.85f) * flash;
			baseColor.z = (1.0f - dangerProgress) * 0.35f;
		}
	}

	std::vector<Vector2> marks;

	if (player) {
		Vector2 pCenter = player->GetCenter();
		marks.push_back({pCenter.x - camera.x, pCenter.y - camera.y});
	}

	for (StitchTarget* target : stitched_) {
		if (!target) {
			continue;
		}
		Vector2 c = AABBCenter(target->GetAABB());
		marks.push_back({c.x - camera.x, c.y - camera.y});
	}

	// 糸を描画（dangerProgress を渡してブレ＆間隔変更を反映）
	for (size_t i = 1; i < marks.size(); ++i) {
		DrawThread(marks[i - 1], marks[i], baseColor, dangerProgress);
	}

	size_t startIndex = (player != nullptr) ? 1 : 0;
	for (size_t i = startIndex; i < marks.size(); ++i) {
		DrawMark(marks[i], baseColor);
	}
}