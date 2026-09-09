#pragma once
#include "StitchTarget.h"
#include <vector>

class MapChipField;
class Player;

class Enemy : public StitchTarget {
public:
	enum class State {
		kPatrol, // 通常：とことこ巡回
		kFlee,   // ピンチ：パニック逃走
	};

	// minX, maxX のデフォルト引数を 0.0f に設定（0 の場合は 1280px 基準で自動設定）
	void Initialize(uint32_t textureHandle, const KamataEngine::Vector2& position, float minX = 0.0f, float maxX = 0.0f);

	// StitchTarget 継承用の単一 Update 関数（Player* のデフォルト値を nullptr に設定）
	void Update(MapChipField* mapChipField, Player* player = nullptr);
	void Update() override { Update(nullptr, nullptr); }

	void Draw(const KamataEngine::Vector2& camera) override;

	AABB2 GetAABB() const override;
	KamataEngine::Vector2 GetPosition() const override { return position_; }
	void SetPosition(const KamataEngine::Vector2& position) override { position_ = position; }

	bool CanStitch() const override { return hp_ > 0; }
	bool IsFixed() const override { return false; }
	float GetMass() const override { return 1.0f; }
	Kind GetKind() const override { return Kind::kFodder; }
	bool IsDead() const override { return hp_ <= 0; }

	void OnCinchHit(int damage) override;
	void ApplyKnockback(const KamataEngine::Vector2& velocity) override;
	int GetHp() const { return hp_; }
	int GetMaxHp() const { return kMaxHp; }
	KamataEngine::Vector2 GetSize() const { return size_; }

	void SetCinching(bool cinching) { isCinching_ = cinching; }
	bool IsCinching() const override { return isCinching_; }

	// 敵同士の衝突処理（重なり解消と反転）
	static void CheckEnemyCollisions(std::vector<Enemy*>& enemies);

private:
	// 画面幅（1280px）を基準に二回り小さい巡回エリアを設定する
	void ResetPatrolRangeToScreen();

private:
	KamataEngine::Sprite* sprite_ = nullptr;
	KamataEngine::Vector2 position_{};
	KamataEngine::Vector2 velocity_{};
	//KamataEngine::Vector2 size_{48.0f, 56.0f};
	KamataEngine::Vector2 size_{60.0f, 80.0f};

	static inline const int kMaxHp = 2;
	int hp_ = kMaxHp;
	int hitFlash_ = 0;
	int patrolDir_ = 1;
	float minX_ = 0.0f;
	float maxX_ = 0.0f;

	State state_ = State::kPatrol;

	int fleeTimer_ = 0;
	static inline const int kFleeDuration = 300; // 5秒（60fps × 5）

	static inline const float kGravity = 0.55f;
	static inline const float kPatrolSpeed = 1.4f;
	static inline const float kFleeSpeed = 3.8f;

	// 2560px 画面基準の定数
	static inline const float kScreenWidth = 2560.0f; // 画面横幅
	static inline const float kMarginX = 100.0f;      // 画面両端からの余白

	bool isCinching_ = false;
};