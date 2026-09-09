#pragma once
#include "AABB2.h"
#include "KamataEngine.h"

class MapChipField;

// プレイヤー用テクスチャハンドル群
struct PlayerTextureHandles {
	uint32_t walkRight = 0;
	uint32_t walkLeft = 0;
	uint32_t attackRight = 0;
	uint32_t attackLeft = 0;
	uint32_t deadRight = 0;
	uint32_t deadLeft = 0;
};

class Player {
public:
	enum class MotionState { kIdle, kWalk, kAttack, kDead };

	void Initialize(const PlayerTextureHandles& handles);
	void Update(MapChipField* mapChipField);
	void Draw(const KamataEngine::Vector2& camera);

	AABB2 GetAABB() const;
	KamataEngine::Vector2 GetPosition() const { return position_; }
	KamataEngine::Vector2 GetCenter() const;
	KamataEngine::Vector2 GetVelocity() const { return velocity_; }

	void SetPosition(const KamataEngine::Vector2& position) { position_ = position; }
	void SetMapBounds(float left, float right);

	bool IsDashing() const { return dashTimer_ > 0; }
	bool IsOnGround() const { return onGround_; }
	bool IsInvincible() const { return invincibleTimer_ > 0; }
	bool InvincibleJustEnded() const { return invincibleJustEnded_; }
	void SetInvincible(int frames);
	int GetHp() const { return hp_; }
	int GetMaxHp() const { return kMaxHp; }
	bool IsDead() const { return hp_ <= 0; }

	// 攻撃処理の呼び出し用
	void TriggerAttack();

	void OnDamaged();
	void Reset(const KamataEngine::Vector2& position);

private:
	void InputMove();
	void UpdateAnimation();

	KamataEngine::Sprite* sprite_ = nullptr;
	KamataEngine::Vector2 position_{};
	KamataEngine::Vector2 velocity_{};
	KamataEngine::Vector2 size_{75.0f, 100.0f};

	int facing_ = 1; // 1: 右向き, -1: 左向き
	bool onGround_ = false;
	int dashTimer_ = 0;
	bool usedAirDash_ = false;

	static inline const int kMaxHp = 3;
	int hp_ = kMaxHp;
	int invincibleTimer_ = 0;
	bool invincibleJustEnded_ = false;

	float mapLeft_ = 0.0f;
	float mapRight_ = 2560.0f;

	// アニメーション関連メンバ変数
	PlayerTextureHandles textures_{};
	MotionState state_ = MotionState::kIdle;
	int animTimer_ = 0;
	int currentFrame_ = 0;
	int attackTimer_ = 0;

	// 1コマあたりの幅と高さ
	static inline const float kFrameWidth = 75.0f;
	static inline const float kFrameHeight = 100.0f;

	static inline const float kAcceleration = 1.2f;
	static inline const float kAttenuation = 0.18f;
	static inline const float kLimitRunSpeed = 6.0f;
	static inline const float kGravity = 0.55f;
	static inline const float kLimitFallSpeed = 14.0f;
	static inline const float kJumpSpeed = -16.0f;
	static inline const float kJumpCut = 0.7f;
	static inline const float kDashSpeed = 16.0f;
	static inline const int kDashDuration = 16;
	static inline const int kInvincibleDuration = 40;
};