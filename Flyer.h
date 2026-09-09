#pragma once
#include "Enemy.h"
#include "StitchTarget.h"
#include <random>
#include <vector>

class MapChipField;
class Player;

class Flyer : public StitchTarget {
public:
	enum class State {
		kPatrol, // 空中巡回
		kCharge, // 曲線ダイブ攻撃
		kReturn  // 攻撃後の復帰
	};

	void Initialize(const EnemyTextureHandles& textures, const KamataEngine::Vector2& position, float minX = 0.0f, float maxX = 0.0f);

	void Update(MapChipField* mapChipField, Player* player = nullptr);
	void Update() override { Update(nullptr, nullptr); }

	void Draw(const KamataEngine::Vector2& camera) override;

	AABB2 GetAABB() const override;
	KamataEngine::Vector2 GetPosition() const override { return position_; }
	void SetPosition(const KamataEngine::Vector2& position) override { position_ = position; }

	bool CanStitch() const override { return hp_ > 0; }
	bool IsFixed() const override { return false; }
	float GetMass() const override { return 1.0f; }
	Kind GetKind() const override { return Kind::kFodder; } // 雑魚敵扱い
	bool IsDead() const override { return hp_ <= 0; }

	void OnCinchHit(int damage) override;
	void ApplyKnockback(const KamataEngine::Vector2& velocity) override;
	int GetHp() const { return hp_; }
	int GetMaxHp() const { return kMaxHp; }
	KamataEngine::Vector2 GetSize() const { return size_; }

private:
	void ResetPatrolRangeToSection();

	void UpdatePatrol(Player* player, float speedMult, float attackIntervalMult);
	void UpdateCharge(Player* player, float speedMult);
	void UpdateReturn(float speedMult);

private:
	KamataEngine::Sprite* sprite_ = nullptr;
	EnemyTextureHandles textures_{}; // 左右のアニメーションテクスチャ
	KamataEngine::Vector2 position_{};
	KamataEngine::Vector2 velocity_{};
	KamataEngine::Vector2 basePatrolPos_{}; // 巡回の基準座標
	KamataEngine::Vector2 size_{50.0f, 50.0f};

	// アニメーション管理用
	int animTimer_ = 0;
	int currentFrame_ = 0;
	bool isFacingLeft_ = true;

	static inline const float kFrameWidth = 50.0f;  // 1コマの幅
	static inline const float kFrameHeight = 50.0f; // 1コマの高さ
	static inline const int kNumFrames = 4;         // 全コマ数 (4枚コマ)
	static inline const int kFrameInterval = 8;     // コマ切り替え速度

	static inline const int kMaxHp = 2;
	int hp_ = kMaxHp;
	int hitFlash_ = 0;
	int direction_ = 1;
	float minX_ = 0.0f;
	float maxX_ = 0.0f;

	State state_ = State::kPatrol;
	float sinAngle_ = 0.0f;
	int attackTimer_ = 0;

	// 曲線突撃用
	KamataEngine::Vector2 chargeStartPos_{};
	KamataEngine::Vector2 chargeTargetPos_{};
	float chargeProgress_ = 0.0f;

	static inline const float kScreenWidth = 2560.0f;
	static inline const float kMarginX = 100.0f;

	int nextAttackInterval_ = 180;

	float spawnY_ = 0.0f; // 初期スポーン時の高度を保持する変数

	bool isCinching_ = false;

	// 乱数生成器
	std::mt19937 randomEngine_{std::random_device{}()};
};