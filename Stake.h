#pragma once
#include "StitchTarget.h"

/// <summary>
/// ボス部屋の杭。縫えるが動かない。ダメージも受けない。
/// </summary>
class Stake : public StitchTarget {
public:
	void Initialize(uint32_t textureHandle, const KamataEngine::Vector2& position);
	void Update() override;
	void Draw(const KamataEngine::Vector2& camera) override;

	AABB2 GetAABB() const override;
	KamataEngine::Vector2 GetPosition() const override { return position_; }
	void SetPosition(const KamataEngine::Vector2& position) override { (void)position; }

	bool CanStitch() const override { return true; }
	bool IsFixed() const override { return true; }
	float GetMass() const override { return 9999.0f; }
	Kind GetKind() const override { return Kind::kStake; }
	bool IsDead() const override { return false; }
	void OnCinchHit(int damage) override { (void)damage; }

private:
	KamataEngine::Sprite* sprite_ = nullptr;
	KamataEngine::Vector2 position_{};
	KamataEngine::Vector2 size_{28.0f, 160.0f};
};