#pragma once
#include "KamataEngine.h"
#include "StitchTarget.h"
#include <vector>

class Player;

/// <summary>
/// 縫いリストと絞りの管理。
/// 糸は自機から伸ばさず、対象の＋マーク同士を点でつなぐ。
/// </summary>
class HookStitch {
public:
	void Initialize(uint32_t textureHandle);
	void Update(Player* player, const std::vector<StitchTarget*>& targets);
	void Draw(Player* player, const KamataEngine::Vector2& camera);

	void TryStitch(StitchTarget* target);
	void Cinch(Player* player);
	void Clear();

	int GetCount() const { return static_cast<int>(stitched_.size()); }
	bool IsCinching() const { return cinchTimer_ > 0; }

private:
	bool Contains(StitchTarget* target) const;
	// プレイヤーの位置も考慮して重心を計算
	KamataEngine::Vector2 CalcCentroid(Player* player) const;
	// プレイヤーとの衝突も判定に含める
	bool CheckCinchCollision(Player* player) const;
	void ResolveCinch(const KamataEngine::Vector2& centroid);
	void DrawThread(const KamataEngine::Vector2& from, const KamataEngine::Vector2& to, const KamataEngine::Vector4& color, float dangerProgress);
	void DrawMark(const KamataEngine::Vector2& center, const KamataEngine::Vector4& color);

	std::vector<StitchTarget*> stitched_;
	std::vector<KamataEngine::Sprite*> dotSprites_;
	std::vector<KamataEngine::Sprite*> markSprites_;
	std::vector<KamataEngine::Sprite*> markCrossSprites_;
	int dotIndex_ = 0;

	int cinchTimer_ = 0;
	bool pendingResolve_ = false; // 絞り中。衝突か時間切れでダメージ
	int markAnim_ = 0;

	static inline const int kMaxStitch = 4;
	static inline const int kDotCount = 512;
	static inline const int kCoolDown = 20;
	static inline const int kCinchDuration = 24;
	static inline const int kCinchInvincible = 22;
	static inline const float kCinchSpeed = 18.0f;

	static inline const float kMaxLinkDistance = 700.0f; // 隣り合う2点間の最大限界距離 (px)
	static inline const float kMaxTotalLength = 2000.0f;  // 糸全体の合計最大限界距離 (px)
};