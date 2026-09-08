#pragma once
#include "StitchTarget.h"

class MapChipField;

class Boss : public StitchTarget {
public:
	void Initialize(uint32_t textureHandle, const KamataEngine::Vector2& position, const KamataEngine::Vector4& color, float minX, float maxX);

	void Update() override { Update(nullptr); }
	void Update(MapChipField* mapChipField);
	void Draw(const KamataEngine::Vector2& camera) override;

	AABB2 GetAABB() const override;
	KamataEngine::Vector2 GetPosition() const override { return position_; }
	void SetPosition(const KamataEngine::Vector2& position) override { position_ = position; }

	bool CanStitch() const override { return hp_ > 0; }
	bool IsFixed() const override { return false; }
	float GetMass() const override { return 3.0f; }
	Kind GetKind() const override { return Kind::kBoss; }
	bool IsDead() const override { return hp_ <= 0; }

	void OnCinchHit(int damage) override;
	int GetHp() const { return hp_; }
	int GetMaxHp() const { return kMaxHp; }
	KamataEngine::Vector2 GetSize() const { return size_; }
	void ApplyKnockback(const KamataEngine::Vector2& velocity) override;

private:
	KamataEngine::Sprite* sprite_ = nullptr;
	KamataEngine::Vector2 position_{};
	KamataEngine::Vector2 velocity_{};
	KamataEngine::Vector2 size_{96.0f, 120.0f};
	KamataEngine::Vector4 color_{1.0f, 0.3f, 0.3f, 1.0f};

	static inline const int kMaxHp = 6;
	int hp_ = kMaxHp;
	int hitFlash_ = 0;
	int patrolDir_ = 1;
	float minX_ = 80.0f;
	float maxX_ = 1100.0f;

	static inline const float kGravity = 0.45f;
	static inline const float kPatrolSpeed = 1.6f;
};