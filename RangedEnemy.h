#pragma once
#include "StitchTarget.h"
#include <vector>

class MapChipField;
class Player;

// 遠距離敵が発射する弾構造体
struct EnemyBullet {
	KamataEngine::Vector2 position;
	KamataEngine::Vector2 velocity;
	KamataEngine::Vector2 size{50.0f, 10.0f}; // 50x10 のサイズ
	bool isAlive = true;

	AABB2 GetAABB() const {
		return {
		    position, {position.x + size.x, position.y + size.y}
        };
	}
};

class RangedEnemy : public StitchTarget {
public:
	void Initialize(uint32_t textureHandle, uint32_t bulletTextureHandle, const KamataEngine::Vector2& position);

	void Update(MapChipField* mapChipField, Player* player = nullptr);
	void Update() override { Update(nullptr, nullptr); }

	void Draw(const KamataEngine::Vector2& camera) override;

	AABB2 GetAABB() const override;
	KamataEngine::Vector2 GetPosition() const override { return position_; }
	void SetPosition(const KamataEngine::Vector2& position) override { position_ = position; }

	bool CanStitch() const override { return hp_ > 0; }
	bool IsFixed() const override { return false; }
	float GetMass() const override { return 1.0f; }
	Kind GetKind() const override { return Kind::kRanged; }
	bool IsDead() const override { return hp_ <= 0; }

	void OnCinchHit(int damage) override;
	void ApplyKnockback(const KamataEngine::Vector2& velocity) override;

	int GetHp() const { return hp_; }
	int GetMaxHp() const { return kMaxHp; }
	KamataEngine::Vector2 GetSize() const { return size_; }

	void SetCinching(bool cinching) { isCinching_ = cinching; }
	bool IsCinching() const override { return isCinching_; }

	// 発射された弾リストの取得（Playerとの当たり判定用）
	std::vector<EnemyBullet>& GetBullets() { return bullets_; }

private:
	void Shoot(Player* player);

private:
	KamataEngine::Sprite* sprite_ = nullptr;
	KamataEngine::Sprite* bulletSprite_ = nullptr;
	uint32_t bulletTextureHandle_ = 0;

	KamataEngine::Vector2 position_{};
	KamataEngine::Vector2 velocity_{};
	KamataEngine::Vector2 size_{60.0f, 80.0f};

	static inline const int kMaxHp = 3;
	int hp_ = kMaxHp;
	int hitFlash_ = 0;
	int moveDir_ = 1; // 1: 右移動, -1: 左移動

	// 移動パラメータ
	static inline const float kGravity = 0.55f;
	static inline const float kNormalSpeed = 1.8f;      // 通常時移動速度
	static inline const float kPinchSpeed = 3.2f;       // ピンチ時移動速度
	static inline const float kTargetDistance = 350.0f; // 維持したい最適距離
	static inline const float kDistanceMargin = 40.0f;  // 許容誤差範囲

	// 射撃・ピンチ設定
	int shotTimer_ = 0;
	static inline const int kPinchHpThreshold = 1;     // HPが1以下でピンチ状態
	static inline const int kNormalShotInterval = 120; // 通常時：2秒に1発 (60fps)
	static inline const int kPinchShotInterval = 60;   // ピンチ時：1秒に1発
	static inline const float kBulletSpeed = 8.0f;     // 弾速

	bool isCinching_ = false;
	std::vector<EnemyBullet> bullets_;
};