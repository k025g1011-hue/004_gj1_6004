#pragma once
#include "BaseScene.h"
#include "Boss.h"
#include "BossAnimation.h"
#include "Camera2D.h"
#include "Enemy.h"
#include "Flyer.h"
#include "RangedEnemy.h"
#include "HeavyEnemy.h"
#include "HookStitch.h"
#include "MapChipField.h"
#include "Player.h"
#include "Stage.h"
#include "Stake.h"
#include <map>
#include <array>
#include <string>
#include <vector>

// 描画用ブロック構造体
struct BlockObject {
	KamataEngine::Sprite* sprite = nullptr;
	KamataEngine::Vector2 position{};
};

// 各敵タイプのテクスチャまとめ
struct AllEnemyTextures {
	EnemyTextureHandles fodder; // 雑魚敵 (60x80 * 4枚)
	EnemyTextureHandles flyer;  // 飛行敵 (50x50 * 4枚)
	EnemyTextureHandles ranged; // 遠距離敵 (60x80 * 4枚)
	EnemyTextureHandles heavy;  // 重装備敵 (75x100 * 4枚)

	EnemyBulletTextureHandles rangedBullet; // 遠距離敵の弾 (左右画像)
};

/// <summary>
/// ゲームプレイシーン（共通基底クラス）
/// </summary>
class GameScene : public BaseScene {
public:
	~GameScene() override;
	void Initialize() override;
	void Update() override;
	void Draw() override;

	// ステージ背景切り替え用関数
	void SetStage(int stageIndex);

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

	void UpdateBossPhase();

	bool showPlayerHp_ = true;
	bool enablePlayerDamage_ = true;

	// ステージ内の雑魚敵が全滅したか確認する判定関数
	bool IsStageCleared() const;

	// ゲームオブジェクト
	Player* player_ = nullptr;
	Boss* bossA_ = nullptr;
	Boss* bossB_ = nullptr;
	// ※ボスが合体した後は bossA_ をそのまま合体ボスとして運用、または独立して管理します
	Stake* stakeL_ = nullptr;
	Stake* stakeR_ = nullptr;
	HookStitch* hookStitch_ = nullptr;
	MapChipField* mapChipField_ = nullptr;

	std::vector<Enemy*> fodder_;
	std::vector<EnemySpawn> fodderSpawns_;

	std::vector<Flyer*> flyers_;
	std::vector<EnemySpawn> flyerSpawns_;

	std::vector<RangedEnemy*> rangedEnemies_;
	std::vector<EnemySpawn> rangedEnemySpawns_;

	std::vector<HeavyEnemy*> heavyEnemies_;
	std::vector<EnemySpawn> heavyEnemySpawns_;

	std::vector<StitchTarget*> targets_;


	// 描画関連
	std::map<MapChipType, uint32_t> blockTextures_; // チップ種別ごとのテクスチャハンドル
	std::vector<BlockObject> blockObjects_;         // 生成された各ブロックのスプライトと座標
	/*std::vector<KamataEngine::Sprite*> blockSprites_;
	std::vector<KamataEngine::Vector2> blockPositions_;*/

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

	static inline const int kMaxRangedUi = 8;
	std::array<KamataEngine::Sprite*, kMaxRangedUi> rangedHpBack_{};
	std::array<KamataEngine::Sprite*, kMaxRangedUi> rangedHpFill_{};

	static inline const int kMaxHeavyUi = 8;
	std::array<KamataEngine::Sprite*, kMaxHeavyUi> heavyHpBack_{};
	std::array<KamataEngine::Sprite*, kMaxHeavyUi> heavyHpFill_{};

	std::array<KamataEngine::Sprite*, 8> doorSprites_{};
	uint32_t whiteTexture_ = 0;

	PlayerTextureHandles playerTextures_{};

	// 敵のリソースハンドルを追加
	AllEnemyTextures enemyTextures_{};

	// ボス用アニメーションリソース一式
	AllBossTextures bossTextures_{};

	// ボスの合体・フェーズ制御フラグ
	bool isBossPhase2_ = false;

	// 全4ステージ (ステージ1~3 + ボスステージ)
	static inline const int kMaxStage = 4;

	// ボスステージのインデックス（呼び出しやすくするため定義）
	static inline const int kBossStageIndex = 3;

	// 各ステージの L / R テクスチャ＆スプライト
	uint32_t bgLeftTextures_[kMaxStage]{};
	uint32_t bgRightTextures_[kMaxStage]{};

	KamataEngine::Sprite* bgLeftSprite_ = nullptr;
	KamataEngine::Sprite* bgRightSprite_ = nullptr;

	int currentStageIndex_ = 0;

	// ステージ・データ状態
	WorldDesc world_{};
	int enterWait_ = 0;

	Camera2D camera_;
};