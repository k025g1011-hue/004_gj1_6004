#pragma once
#include "StitchTarget.h"
#include <vector>

class MapChipField;
class Player;

/// <summary>
/// 大型重装備敵（75x100）
/// 通常は巡回し、視界に入ると2回小ジャンプの合図後に突撃。
/// ピンチ状態（HP減少）で移動・突撃スピードがアップする。
/// 糸で手繰り寄せられても動かない不動（IsFixed = true）属性を持つ。
/// </summary>
class HeavyEnemy : public StitchTarget {
public:
	enum class State {
		kPatrol,  // 通常巡回
		kWarning, // 突撃前兆（2回小ジャンプ）
		kCharge,  // 突撃
		kCooldown // 激突後の隙
	};

	void Initialize(uint32_t textureHandle, const KamataEngine::Vector2& position);

	// StitchTarget 継承用の Update 関数（Player* のデフォルト値を nullptr に設定）
	void Update(MapChipField* mapChipField, Player* player = nullptr);
	void Update() override { Update(nullptr, nullptr); }

	void Draw(const KamataEngine::Vector2& camera) override;

	AABB2 GetAABB() const override;
	KamataEngine::Vector2 GetPosition() const override { return position_; }
	void SetPosition(const KamataEngine::Vector2& position) override { position_ = position; }

	bool CanStitch() const override { return hp_ > 0; }
	// 不動属性（手繰り寄せられても引き寄せられない支柱として機能する）
	bool IsFixed() const override { return true; }
	float GetMass() const override { return 3.0f; } // 大型敵なので質量高め
	Kind GetKind() const override { return Kind::kHeavy; }
	bool IsDead() const override { return hp_ <= 0; }

	void OnCinchHit(int damage) override;
	void ApplyKnockback(const KamataEngine::Vector2& velocity) override;
	int GetHp() const { return hp_; }
	int GetMaxHp() const { return kMaxHp; }
	KamataEngine::Vector2 GetSize() const { return size_; }

	void SetCinching(bool cinching) { isCinching_ = cinching; }
	bool IsCinching() const override { return isCinching_; }

	// 画面基準の巡回エリア設定関数
	void ResetPatrolRangeToScreen();

private:
	KamataEngine::Sprite* sprite_ = nullptr;
	KamataEngine::Vector2 position_{};
	KamataEngine::Vector2 velocity_{};
	KamataEngine::Vector2 size_{75.0f, 100.0f}; // 75x100 の大型サイズ

	static inline const int kMaxHp = 4;
	int hp_ = kMaxHp;
	int hitFlash_ = 0;

	State state_ = State::kPatrol;

	// 巡回エリア用
	float patrolLeft_ = 0.0f;
	float patrolRight_ = 0.0f;
	int dir_ = 1; // 1: 右, -1: 左

	// タイマー関連
	int warningTimer_ = 0;
	int jumpCount_ = 0;
	int cooldownTimer_ = 0;

	static inline const float kGravity = 0.55f;

	bool isCinching_ = false;

	static inline const float kScreenWidth = 2560.0f; // 画面横幅
	static inline const float kMarginX = 100.0f;      // 画面両端からの余白

	int chargeTimer_ = 0;
	bool isBraking_ = false;
};