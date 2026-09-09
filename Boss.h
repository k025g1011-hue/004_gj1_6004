#pragma once
#include "BossAnimation.h"
#include "KamataEngine.h"
#include "StitchTarget.h"
#include <vector>

class MapChipField;
class Player;

// 落下ボタン構造体（第2フェーズ用：40x40）
struct FallingButton {
	KamataEngine::Vector2 position{};
	KamataEngine::Vector2 velocity{};
	KamataEngine::Vector2 size{40.0f, 40.0f};
	bool isAlive = true;

	AABB2 GetAABB() const {
		return {
		    position, {position.x + size.x, position.y + size.y}
        };
	}
};

class Boss : public StitchTarget {
public:
	// フェーズ定義
	enum class Phase {
		kPhase1_TwoBosses, // 第1フェーズ（2体分離）
		kPhase2_Combined   // 第2フェーズ（合体）
	};

	// 行動ステート
	enum class State {
		kIdle,
		kWalk,
		kPunch,      // パンチ（前方に判定を伸張）
		kCharge,     // 突進（第1フェーズ）
		kPress,      // ジャンププレス
		kRainButtons // ボタン降らし（第2フェーズ）
	};

	// ボスの個体ID
	enum class BossID { kBossA, kBossB, kCombined };

	~Boss() override;

	// 初期化
	void Initialize(BossID id, const BossTextureSet& textures, const KamataEngine::Vector2& position, uint32_t buttonTexture = 0);

	void Update() override { Update(nullptr, nullptr); }
	void Update(MapChipField* mapChipField, Player* player = nullptr);
	void Draw(const KamataEngine::Vector2& camera) override;

	// StitchTarget 継承メンバ
	AABB2 GetAABB() const override;
	KamataEngine::Vector2 GetPosition() const override { return position_; }
	void SetPosition(const KamataEngine::Vector2& position) override { position_ = position; }

	bool CanStitch() const override { return hp_ > 0; }
	bool IsFixed() const override { return true; } // 重厚なボスのため固定
	float GetMass() const override { return 10.0f; }
	Kind GetKind() const override { return Kind::kBoss; }
	bool IsDead() const override { return hp_ <= 0; }

	void OnCinchHit(int damage) override;
	void ApplyKnockback(const KamataEngine::Vector2& velocity) override;

	// 情報取得用
	int GetHp() const { return hp_; }
	int GetMaxHp() const { return maxHp_; }
	KamataEngine::Vector2 GetSize() const { return size_; }
	Phase GetPhase() const { return phase_; }

	// 攻撃判定AABBの取得
	AABB2 GetAttackAABB() const;
	bool IsAttacking() const { return isAttacking_; }

	// 落下ボタン（弾）のリスト取得
	const std::vector<FallingButton>& GetButtons() const { return buttons_; }
	std::vector<FallingButton>& GetButtons() { return buttons_; }

	// 合体処理
	void CombineTo(const KamataEngine::Vector2& combinePos, const BossTextureSet& combinedTextures);

	// ★ デバッグ用の枠線描画（緑＝200x300本体枠、赤＝実判定AABB）
	void DrawDebugFrame(const KamataEngine::Vector2& camera);

	// ★ 当たり判定のサイズ調整用（例: 左右20px、上下10px縮小）
	void SetBoxMargin(const KamataEngine::Vector2& margin) { boxMargin_ = margin; }

private:
	void UpdateAnimation();
	void ProcessAI(Player* player);

private:
	BossID id_ = BossID::kBossA;
	Phase phase_ = Phase::kPhase1_TwoBosses;
	State state_ = State::kIdle;

	BossTextureSet textures_{};
	KamataEngine::Sprite* sprite_ = nullptr;

	KamataEngine::Vector2 position_{};
	KamataEngine::Vector2 velocity_{};
	KamataEngine::Vector2 size_{200.0f, 300.0f}; // 200x300 固定

	int maxHp_ = 20;
	int hp_ = 20;
	int hitFlash_ = 0;

	// 行動制御用
	int stateTimer_ = 0;
	int attackCooldown_ = 0;
	bool isFacingLeft_ = true;
	bool isAttacking_ = false;

	// 攻撃判定サイズ・オフセット
	KamataEngine::Vector2 attackSize_{0.0f, 0.0f};

	// 落下ボタン管理（40x40）
	std::vector<FallingButton> buttons_;
	uint32_t buttonTexture_ = 0;
	KamataEngine::Sprite* buttonSprite_ = nullptr;

	// UVアニメーション（200x300コマ）
	static inline const float kFrameWidth = 200.0f;
	static inline const float kFrameHeight = 300.0f;
	static inline const int kFrameInterval = 8;

	int animTimer_ = 0;
	int currentFrame_ = 0;
	int maxFrames_ = 4;

	// ★ 当たり判定調整マージン（初期値：左右20px、上下10px縮小）
	KamataEngine::Vector2 boxMargin_ = {20.0f, 10.0f};

	// ★ デバッグ線用スプライト（緑4本 ＋ 赤4本 ＝ 8本）
	KamataEngine::Sprite* debugLines_[8] = {nullptr};
};