#pragma once
#include "AABB2.h"
#include "KamataEngine.h"

class MapChipField;

class Player {
public:
	void Initialize(uint32_t textureHandle);
	void Update(MapChipField* mapChipField);
	void Draw(const KamataEngine::Vector2& camera);

	AABB2 GetAABB() const;
	KamataEngine::Vector2 GetPosition() const { return position_; }
	KamataEngine::Vector2 GetCenter() const;
	void SetPosition(const KamataEngine::Vector2& position) { position_ = position; }
	void SetMapBounds(float left, float right);

	bool IsDashing() const { return dashTimer_ > 0; }
	bool IsInvincible() const { return invincibleTimer_ > 0; }
	bool InvincibleJustEnded() const { return invincibleJustEnded_; }
	void SetInvincible(int frames);
	int GetHp() const { return hp_; }
	int GetMaxHp() const { return kMaxHp; }
	bool IsDead() const { return hp_ <= 0; }

	void OnDamaged();
	void Reset(const KamataEngine::Vector2& position);

private:
	void InputMove();

	KamataEngine::Sprite* sprite_ = nullptr;
	KamataEngine::Vector2 position_{};
	KamataEngine::Vector2 velocity_{};
	KamataEngine::Vector2 size_{48.0f, 64.0f};

	int facing_ = 1;
	bool onGround_ = false;
	int dashTimer_ = 0;
	bool usedAirDash_ = false;

	static inline const int kMaxHp = 3;
	int hp_ = kMaxHp;
	int invincibleTimer_ = 0;
	bool invincibleJustEnded_ = false;

	float mapLeft_ = 0.0f;
	float mapRight_ = 1280.0f;

	static inline const float kAcceleration = 1.2f;
	static inline const float kAttenuation = 0.18f;
	static inline const float kLimitRunSpeed = 6.0f;
	static inline const float kGravity = 0.55f;
	static inline const float kLimitFallSpeed = 14.0f;
	static inline const float kJumpSpeed = -13.0f;
	static inline const float kJumpCut = 0.45f;
	static inline const float kDashSpeed = 16.0f;
	static inline const int kDashDuration = 16;
	static inline const int kInvincibleDuration = 40;
};