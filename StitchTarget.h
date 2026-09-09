#pragma once
#include "AABB2.h"
#include "KamataEngine.h"

/// <summary>
/// 縫える対象の共通インタフェース。
/// ボス・杭・雑魚・飛行・遠距離敵がこれを継承する。
/// </summary>
class StitchTarget {
public:
	enum class Kind {
		kBoss,
		kStake,
		kFodder,
		kFlyer,
		kRanged,
		kHeavy,
	};

	virtual ~StitchTarget() = default;

	virtual void Update() = 0;
	// camera はワールド→スクリーン変換用
	virtual void Draw(const KamataEngine::Vector2& camera) = 0;

	virtual AABB2 GetAABB() const = 0;
	virtual KamataEngine::Vector2 GetPosition() const = 0;
	virtual void SetPosition(const KamataEngine::Vector2& position) = 0;


	virtual bool CanStitch() const = 0;
	virtual bool IsFixed() const = 0; // 杭など、絞っても動かない
	virtual float GetMass() const = 0;
	virtual Kind GetKind() const = 0;
	virtual bool IsDead() const = 0;

	virtual void OnCinchHit(int damage) = 0;
	virtual void ApplyKnockback(const KamataEngine::Vector2& velocity) { (void)velocity; }

	bool IsStitchCooling() const { return stitchCoolDown_ > 0; }
	virtual bool IsCinching() const { return false; }
	virtual void SetCinching(bool cinching) { (void)cinching; }
	void StartStitchCoolDown(int frames) { stitchCoolDown_ = frames; }
	void UpdateStitchCoolDown() {
		if (stitchCoolDown_ > 0) {
			--stitchCoolDown_;
		}
	}

protected:
	int stitchCoolDown_ = 0; // 同じ対象を連打で縫わないための待ち
};