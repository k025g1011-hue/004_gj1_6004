#pragma once
#include "BaseScene.h"
#include "Boss.h"
#include "Camera2D.h"
#include "Enemy.h"
#include "Flyer.h"
#include "HookStitch.h"
#include "MapChipField.h"
#include "Player.h"
#include "Stage.h"
#include "Stake.h"
#include <array>
#include <string>
#include <vector>

/// <summary>
/// ゲームプレイシーン（共通基底クラス）
/// </summary>
class GameScene : public BaseScene {
public:
	~GameScene() override;
	void Initialize() override;
	void Update() override;
	void Draw() override;

protected:
	// 引数で各ステージの CSV パスを受け取って構築
	void BuildWorld(const std::string& csvPath);
	void ClearCurrentActors();
	void CheckStitchOverlaps();
	void CheckPlayerHits();
	void PushPlayerOutOfBosses();
	void TryEnterNextArea();
	void UpdateCamera();
	void DrawHp();
	void DrawDoors();
	void DrawBlocks();

	// ステージ内の雑魚敵が全滅したか確認する判定関数
	bool IsStageCleared() const;

	// ゲームオブジェクト
	Player* player_ = nullptr;
	Boss* bossA_ = nullptr;
	Boss* bossB_ = nullptr;
	Stake* stakeL_ = nullptr;
	Stake* stakeR_ = nullptr;
	HookStitch* hookStitch_ = nullptr;
	MapChipField* mapChipField_ = nullptr;

	std::vector<Enemy*> fodder_;
	std::vector<EnemySpawn> fodderSpawns_;
	std::vector<Flyer*> flyers_;
	std::vector<EnemySpawn> flyerSpawns_;
	std::vector<StitchTarget*> targets_;

	// 描画関連
	std::vector<KamataEngine::Sprite*> blockSprites_;
	std::vector<KamataEngine::Vector2> blockPositions_;

	KamataEngine::Sprite* backSprite_ = nullptr;
	std::array<KamataEngine::Sprite*, 3> playerHpSprites_{};
	KamataEngine::Sprite* bossAHpBack_ = nullptr;
	KamataEngine::Sprite* bossAHpFill_ = nullptr;
	KamataEngine::Sprite* bossBHpBack_ = nullptr;
	KamataEngine::Sprite* bossBHpFill_ = nullptr;

	static inline const int kMaxFodderUi = 8;
	std::array<KamataEngine::Sprite*, kMaxFodderUi> fodderHpBack_{};
	std::array<KamataEngine::Sprite*, kMaxFodderUi> fodderHpFill_{};

	static inline const int kMaxFlyerUi = 8;
	std::array<KamataEngine::Sprite*, kMaxFlyerUi> flyerHpBack_{};
	std::array<KamataEngine::Sprite*, kMaxFlyerUi> flyerHpFill_{};

	std::array<KamataEngine::Sprite*, 8> doorSprites_{};
	uint32_t whiteTexture_ = 0;

	// ステージ・データ状態
	WorldDesc world_{};
	int enterWait_ = 0;

	Camera2D camera_;
};