#include "GameScene.h"
#include <cmath>

using namespace KamataEngine;

GameScene::~GameScene() {
	ClearCurrentActors();
	delete bgLeftSprite_;
	delete bgRightSprite_;
	delete player_;
	delete hookStitch_;
	delete mapChipField_;

	delete backSprite_;
	for (int i = 0; i < 3; ++i) {
		delete playerHpSprites_[i];
	}
	for (int i = 0; i < 8; ++i) {
		delete doorSprites_[i];
	}
	delete bossAHpBack_;
	delete bossAHpFill_;
	delete bossBHpBack_;
	delete bossBHpFill_;
	for (int i = 0; i < kMaxFodderUi; ++i) {
		delete fodderHpBack_[i];
		delete fodderHpFill_[i];
	}
	for (int i = 0; i < kMaxFlyerUi; ++i) {
		delete flyerHpBack_[i];
		delete flyerHpFill_[i];
	}

	for (int i = 0; i < kMaxRangedUi; ++i) {
		delete rangedHpBack_[i];
		delete rangedHpFill_[i];
	}

	for (int i = 0; i < kMaxHeavyUi; ++i) {
		delete heavyHpBack_[i];
		delete heavyHpFill_[i];
	}
}

void GameScene::ClearCurrentActors() {
	delete bossA_;
	bossA_ = nullptr;
	delete bossB_;
	bossB_ = nullptr;

	delete stakeL_;
	stakeL_ = nullptr;
	delete stakeR_;
	stakeR_ = nullptr;

	for (Enemy* enemy : fodder_) {
		delete enemy;
	}
	fodder_.clear();
	fodderSpawns_.clear();

	for (Flyer* flyer : flyers_) {
		delete flyer;
	}
	flyers_.clear();
	flyerSpawns_.clear();

	for (RangedEnemy* ranged : rangedEnemies_) {
		delete ranged;
	}
	rangedEnemies_.clear();
	rangedEnemySpawns_.clear();

	for (HeavyEnemy* heavy : heavyEnemies_) {
		delete heavy;
	}
	heavyEnemies_.clear();
	heavyEnemySpawns_.clear();

	// 縫い合わせ・攻撃対象リストのクリア
	targets_.clear();
	world_.doors.clear();

	for (auto& block : blockObjects_) {
		delete block.sprite;
	}
	blockObjects_.clear();
}

void GameScene::Initialize() {
	isFinished_ = false;

	whiteTexture_ = TextureManager::Load("white.png");

	// ステージ1用（積み木風ブロックなど）
	blockTextures_[MapChipType::kBuildingBlocksC] = TextureManager::Load("./Resources/blocks/building_c.png");
	blockTextures_[MapChipType::kBuildingBlocksL] = TextureManager::Load("./Resources/blocks/building_l.png");
	blockTextures_[MapChipType::kBuildingBlocksR] = TextureManager::Load("./Resources/blocks/building_r.png");

	// ステージ2用（レゴ風ブロックなど）
	blockTextures_[MapChipType::kLegoBlocksC] = TextureManager::Load("./Resources/blocks/lego_c.png");
	blockTextures_[MapChipType::kLegoBlocksL] = TextureManager::Load("./Resources/blocks/lego_l.png");
	blockTextures_[MapChipType::kLegoBlocksR] = TextureManager::Load("./Resources/blocks/lego_r.png");

	// ステージ3用（ボタン風ブロックなど）
	blockTextures_[MapChipType::kButtonBlocksC] = TextureManager::Load("./Resources/blocks/button_c.png");
	blockTextures_[MapChipType::kButtonBlocksL] = TextureManager::Load("./Resources/blocks/button_l.png");
	blockTextures_[MapChipType::kButtonBlocksR] = TextureManager::Load("./Resources/blocks/button_r.png");

	// --- ステージ1~3 の背景読み込み ---
	bgLeftTextures_[0] = TextureManager::Load("./Resources/stage/Stage1_L.png");
	bgRightTextures_[0] = TextureManager::Load("./Resources/stage/Stage1_R.png");

	bgLeftTextures_[1] = TextureManager::Load("./Resources/stage/Stage2_L.png");
	bgRightTextures_[1] = TextureManager::Load("./Resources/stage/Stage2_R.png");

	bgLeftTextures_[2] = TextureManager::Load("./Resources/stage/Stage3_L.png");
	bgRightTextures_[2] = TextureManager::Load("./Resources/stage/Stage3_R.png");

	// ボスステージの背景読み込み
	
	bgLeftTextures_[kBossStageIndex]  = TextureManager::Load("./Resources/stage/StageBoss_L.png");
	bgRightTextures_[kBossStageIndex] = TextureManager::Load("./Resources/stage/StageBoss_R.png");
	

	// スプライト生成は SetStage(index) で行うため、ここではポインタクリアのみ
	bgLeftSprite_ = nullptr;
	bgRightSprite_ = nullptr;

	// プレイヤー画像（スプライトシート）の読み込み
	playerTextures_.walkRight = TextureManager::Load("./Resources/player/PlayerWalk_R.png");
	playerTextures_.walkLeft = TextureManager::Load("./Resources/player/PlayerWalk_L.png");
	playerTextures_.attackRight = TextureManager::Load("./Resources/player/PlayerAttack_R.png");
	playerTextures_.attackLeft = TextureManager::Load("./Resources/player/PlayerAttack_L.png");
	playerTextures_.deadRight = TextureManager::Load("./Resources/player/PlayerDeath_R.png");
	playerTextures_.deadLeft = TextureManager::Load("./Resources/player/PlayerDeath_L.png");

	// 1. 雑魚敵 (60x80 * 4枚)
	enemyTextures_.fodder.left = TextureManager::Load("./Resources/enemy/fodder_L.png");
	enemyTextures_.fodder.right = TextureManager::Load("./Resources/enemy/fodder_R.png");

	// 2. 飛行敵 (50x50 * 4枚)
	enemyTextures_.flyer.left = TextureManager::Load("./Resources/enemy/flyer_L.png");
	enemyTextures_.flyer.right = TextureManager::Load("./Resources/enemy/flyer_R.png");

	// 3. 遠距離敵 (60x80 * 4枚)
	enemyTextures_.ranged.left = TextureManager::Load("./Resources/enemy/ranged_L.png");
	enemyTextures_.ranged.right = TextureManager::Load("./Resources/enemy/ranged_R.png");

	// 4. 重装備敵 (75x100 * 4枚)
	enemyTextures_.heavy.left = TextureManager::Load("./Resources/enemy/heavy_L.png");
	enemyTextures_.heavy.right = TextureManager::Load("./Resources/enemy/heavy_R.png");

	enemyTextures_.rangedBullet.left = TextureManager::Load("./Resources/enemy/Needle_L.png");
	enemyTextures_.rangedBullet.right = TextureManager::Load("./Resources/enemy/Needle_R.png");

	// --- 1. ボスA (第1フェーズ) テクスチャ読み込み ---
	// 歩き (800x300 - 4コマ)
	bossTextures_.bossA.walk.left = TextureManager::Load("./Resources/boss/BlackBoss_L.png");
	bossTextures_.bossA.walk.right = TextureManager::Load("./Resources/boss/BlackBoss_R.png");

	// パンチ (800x300 - 4コマ)
	bossTextures_.bossA.punch.left = TextureManager::Load("./Resources/boss/BlackBossPunch_L.png");
	bossTextures_.bossA.punch.right = TextureManager::Load("./Resources/boss/BlackBossPunch_R.png");

	// ジャンププレス (1600x300 - 8コマ)
	bossTextures_.bossA.press = TextureManager::Load("./Resources/boss/BlackBossPress.png");

	// --- 2. ボスB (第1フェーズ) テクスチャ読み込み ---
	// 歩き (800x300 - 4コマ)
	bossTextures_.bossB.walk.left = TextureManager::Load("./Resources/boss/PinkBoss_L.png");
	bossTextures_.bossB.walk.right = TextureManager::Load("./Resources/boss/PinkBoss_R.png");

	// パンチ (800x300 - 4コマ)
	bossTextures_.bossB.punch.left = TextureManager::Load("./Resources/boss/PinkBossPunch_L.png");
	bossTextures_.bossB.punch.right = TextureManager::Load("./Resources/boss/PinkBossPunch_R.png");

	// ジャンププレス (1600x300 - 8コマ)
	bossTextures_.bossB.press = TextureManager::Load("./Resources/boss/PinkBossPress.png");

	// --- 3. 合体ボス (第2フェーズ) テクスチャ読み込み ---
	// 歩き (800x300 - 4コマ)
	bossTextures_.combined.walk.left = TextureManager::Load("./Resources/boss/BBoss_L.png");
	bossTextures_.combined.walk.right = TextureManager::Load("./Resources/boss/BBoss_R.png");

	// パンチ (800x300 - 4コマ)
	bossTextures_.combined.punch.left = TextureManager::Load("./Resources/boss/BBossPunch_L.png");
	bossTextures_.combined.punch.right = TextureManager::Load("./Resources/boss/BBossPunch_R.png");

	// ジャンププレス (1600x300 - 8コマ)
	bossTextures_.combined.press = TextureManager::Load("./Resources/boss/BBossPress.png");

	// --- 4. 落下物 (40x40 ボタン) テクスチャ読み込み ---
	bossTextures_.buttonTexture = TextureManager::Load("./Resources/boss/FallingObject.png");

	backSprite_ = Sprite::Create(whiteTexture_, {0.0f, 0.0f});

	for (int i = 0; i < 3; ++i) {
		playerHpSprites_[i] = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	}
	for (int i = 0; i < 8; ++i) {
		doorSprites_[i] = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	}
	bossAHpBack_ = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	bossAHpFill_ = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	bossBHpBack_ = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	bossBHpFill_ = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	for (int i = 0; i < kMaxFodderUi; ++i) {
		fodderHpBack_[i] = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
		fodderHpFill_[i] = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	}

	for (int i = 0; i < kMaxFlyerUi; ++i) {
		flyerHpBack_[i] = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
		flyerHpFill_[i] = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	}

	for (int i = 0; i < kMaxRangedUi; ++i) {
		rangedHpBack_[i] = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
		rangedHpFill_[i] = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	}

	for (int i = 0; i < kMaxHeavyUi; ++i) {
		heavyHpBack_[i] = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
		heavyHpFill_[i] = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	}

	player_ = new Player();
	player_->Initialize(playerTextures_);
	hookStitch_ = new HookStitch();
	hookStitch_->Initialize(whiteTexture_);

	mapChipField_ = new MapChipField();
}

void GameScene::SetStage(int stageIndex) {
	if (stageIndex < 0 || stageIndex >= kMaxStage) {
		return;
	}

	currentStageIndex_ = stageIndex;

	// 古いスプライトを解放
	delete bgLeftSprite_;
	delete bgRightSprite_;

	// 指定ステージのテクスチャで新規生成
	bgLeftSprite_ = Sprite::Create(bgLeftTextures_[currentStageIndex_], {0.0f, 0.0f});
	bgRightSprite_ = Sprite::Create(bgRightTextures_[currentStageIndex_], {1280.0f, 0.0f});
}

void GameScene::BuildWorld(const std::string& csvPath) {
	// 前回生成したブロックオブジェクト（スプライト）を解放・クリア
	for (auto& block : blockObjects_) {
		delete block.sprite;
	}
	blockObjects_.clear();

	ClearCurrentActors();
	if (hookStitch_) {
		hookStitch_->Clear();
	}

	world_ = GetWorldDesc();
	world_.doors.clear();

	// 指定された CSV パスを読み込む
	mapChipField_->LoadMapChipCsv(csvPath);

	int bossCount = 0;
	Vector2 playerSpawnPos = {100.0f, 500.0f}; // マップ上に配置がない場合の初期予備座標
	bool foundPlayerSpawn = false;

	const uint32_t h = mapChipField_->GetNumBlockHorizontal();
	const uint32_t v = mapChipField_->GetNumBlockVertical();

	for (uint32_t y = 0; y < v; ++y) {
		for (uint32_t x = 0; x < h; ++x) {
			const int ix = static_cast<int>(x);
			const int iy = static_cast<int>(y);
			const MapChipType type = mapChipField_->GetMapChipTypeByIndex(ix, iy);
			const Vector2 cell = mapChipField_->GetMapChipPositionByIndex(ix, iy);

			// 1（通常ブロック）および 10〜18（一方向ブロック）の生成処理
			if (type == MapChipType::kBlock) {
				continue;
			} else if (blockTextures_.count(type) > 0) {
				// 10〜18 の追加ブロック（指定画像で描画する場合）
				BlockObject obj;
				obj.position = cell;
				obj.sprite = Sprite::Create(blockTextures_[type], cell);
				blockObjects_.push_back(obj);
			} else if (type == MapChipType::kPlayer) {
				if (!foundPlayerSpawn) {
					playerSpawnPos = cell;
					// プレイヤー（高さ100px）の足元合わせ
					playerSpawnPos.y = mapChipField_->SnapFeetToFloor(cell.x + 10.0f, 100.0f);
					foundPlayerSpawn = true;
				}
			} else if (type == MapChipType::kFodder) {
				EnemySpawn spawn;
				spawn.position = cell;
				spawn.position.y = mapChipField_->SnapFeetToFloor(cell.x + 10.0f, 80.0f);

				Enemy* enemy = new Enemy();
				enemy->Initialize(enemyTextures_.fodder, spawn.position);
				fodder_.push_back(enemy);
				fodderSpawns_.push_back(spawn);
				targets_.push_back(enemy);

			} else if (type == MapChipType::kFlyer) {
				EnemySpawn spawn;
				spawn.position = cell; // 空中の高さを維持

				Flyer* flyer = new Flyer();
				flyer->Initialize(enemyTextures_.flyer, spawn.position);
				flyers_.push_back(flyer);
				flyerSpawns_.push_back(spawn);
				targets_.push_back(flyer);
			} else if (type == MapChipType::kRanged) {
				Vector2 spawn = cell;
				spawn.y = mapChipField_->SnapFeetToFloor(cell.x + 10.0f, 80.0f);

				RangedEnemy* ranged = new RangedEnemy();
				ranged->Initialize(enemyTextures_.ranged, enemyTextures_.rangedBullet, spawn);
				rangedEnemies_.push_back(ranged);
				targets_.push_back(ranged);
			} else if (type == MapChipType::kHeavy) {
				EnemySpawn spawn;
				spawn.position = cell;
				spawn.position.y = mapChipField_->SnapFeetToFloor(cell.x + 10.0f, 100.0f);

				HeavyEnemy* heavy = new HeavyEnemy();
				heavy->Initialize(enemyTextures_.heavy, spawn.position);
				heavyEnemies_.push_back(heavy);
				heavyEnemySpawns_.push_back(spawn);
				targets_.push_back(heavy);
			} else if (type == MapChipType::kDoor) {
				DoorDesc door;
				door.x = cell.x;
				door.y = cell.y;
				door.width = MapChipField::kTileWidth;
				door.height = MapChipField::kTileHeight;
				world_.doors.push_back(door);
			} else if (type == MapChipType::kStake) {
				Vector2 spawn = cell;
				spawn.y = mapChipField_->SnapFeetToFloor(cell.x + 8.0f, 160.0f);
				Stake* stake = new Stake();
				stake->Initialize(whiteTexture_, spawn);
				if (!stakeL_) {
					stakeL_ = stake;
				} else if (!stakeR_) {
					stakeR_ = stake;
				}
				targets_.push_back(stake);
			} else if (type == MapChipType::kBoss) {
				Vector2 spawn = cell;
				// ボスの高さ（300.0f）に合わせて足元を接地
				spawn.y = mapChipField_->SnapFeetToFloor(cell.x + 8.0f, 300.0f);

				Boss* boss = new Boss();
				if (bossCount == 0) {
					// 1体目: ボスA（黒）
					boss->Initialize(Boss::BossID::kBossA, bossTextures_.bossA, spawn, bossTextures_.buttonTexture);
					bossA_ = boss;
				} else {
					// 2体目: ボスB（ピンク）
					boss->Initialize(Boss::BossID::kBossB, bossTextures_.bossB, spawn, bossTextures_.buttonTexture);
					bossB_ = boss;
				}
				++bossCount;
				targets_.push_back(boss);
			}
		}
	}

	// ★ ２体のボスが生成されたら、お互いを「相方」として登録！
	if (bossA_ && bossB_) {
		bossA_->SetPartner(bossB_);
		bossB_->SetPartner(bossA_);
	}

	// マップの横幅を計算してプレイヤーの移動限界（Bounds）を設定
	const float mapWidth = static_cast<float>(h) * MapChipField::kTileWidth;
	player_->SetMapBounds(0.0f, mapWidth);
	player_->Reset(playerSpawnPos);

	enterWait_ = 30;
	UpdateCamera();
}

void GameScene::UpdateCamera() {
	const float kScreenWidth = 1280.0f;

	float mapWidth = kScreenWidth;
	if (mapChipField_) {
		mapWidth = static_cast<float>(mapChipField_->GetNumBlockHorizontal()) * MapChipField::kTileWidth;
	}

	float targetX = 0.0f;
	if (player_) {
		targetX = player_->GetCenter().x - (kScreenWidth * 0.5f);
	}

	float minX = 0.0f;
	float maxX = (mapWidth > kScreenWidth) ? (mapWidth - kScreenWidth) : 0.0f;

	if (targetX < minX)
		targetX = minX;
	if (targetX > maxX)
		targetX = maxX;

	camera_.SetFixed({targetX, 0.0f});
}

// ステージ内の敵（地上敵・飛行敵）が全滅したか判定
bool GameScene::IsStageCleared() const {
	for (const Enemy* enemy : fodder_) {
		if (enemy && !enemy->IsDead()) {
			return false;
		}
	}
	// 飛行敵の生存チェックを追加
	for (const Flyer* flyer : flyers_) {
		if (flyer && !flyer->IsDead()) {
			return false;
		}
	}

	for (const RangedEnemy* ranged : rangedEnemies_) {
		if (ranged && !ranged->IsDead()) {
			return false;
		}
	}

	for (const HeavyEnemy* heavy : heavyEnemies_) {
		if (heavy && !heavy->IsDead()) {
			return false;
		}
	}

	return true;
}

void GameScene::TryEnterNextArea() {
	if (enterWait_ > 0)
		return;

	if (!IsStageCleared())
		return;

	const Vector2 center = player_->GetCenter(); // プレイヤーの中心
	for (const DoorDesc& door : world_.doors) {
		const float doorCenterX = door.x + door.width * 0.5f;
		const float doorCenterY = door.y + door.height * 0.5f;

		// 判定サイズを 0.25f (16px) から 0.75f (48px) に緩和
		const float hitW = door.width * 0.25f;
		const float hitH = door.height * 0.75f;

		if (std::abs(center.x - doorCenterX) <= hitW && std::abs(center.y - doorCenterY) <= hitH) {
			isFinished_ = true;
			break;
		}
	}
}

void GameScene::CheckStitchOverlaps() {
	if (player_->IsInvincible()) {
		return;
	}
	AABB2 playerAABB = player_->GetAABB();
	for (StitchTarget* target : targets_) {
		if (!target || !target->CanStitch() || target->IsDead()) {
			continue;
		}
		if (IsCollision(playerAABB, target->GetAABB())) {
			hookStitch_->TryStitch(target);
		}
	}
}

void GameScene::CheckPlayerHits() {
	if (!enablePlayerDamage_) {
		return;
	}
	if (!player_ || player_->IsDead() || player_->IsInvincible() || player_->IsDashing() || hookStitch_->IsCinching()) {
		return;
	}
	AABB2 playerAABB = player_->GetAABB();
	for (StitchTarget* target : targets_) {
		if (!target || target->IsDead()) {
			continue;
		}
		if (target->GetKind() != StitchTarget::Kind::kBoss && target->GetKind() != StitchTarget::Kind::kFodder && target->GetKind() != StitchTarget::Kind::kFlyer &&
		    target->GetKind() != StitchTarget::Kind::kRanged && target->GetKind() != StitchTarget::Kind::kHeavy) {
			continue;
		}
		if (IsCollision(playerAABB, target->GetAABB())) {
			player_->OnDamaged();
			hookStitch_->Clear();
			break;
		}
	}
	for (RangedEnemy* ranged : rangedEnemies_) {
		if (!ranged || ranged->IsDead()) {
			continue;
		}
		for (auto& bullet : ranged->GetBullets()) {
			if (!bullet.isAlive) {
				continue;
			}
			if (IsCollision(playerAABB, bullet.GetAABB())) {
				bullet.isAlive = false;
				player_->OnDamaged();
				hookStitch_->Clear();
				return;
			}
		}
	}
}
void GameScene::UpdateBossPhase() {
	if (isBossPhase2_) {
		return;
	}

	// ボスが両方存在しない場合はスキップ
	if (!bossA_ || !bossB_) {
		return;
	}

	// 1. どちらかが死亡したか確認
	bool isBossADead = bossA_->IsDead();
	bool isBossBDead = bossB_->IsDead();

	// 2. どちらかのHPが半分以下になったか確認
	bool isBossAHalf = (bossA_->GetHp() <= bossA_->GetMaxHp() / 2);
	bool isBossBHalf = (bossB_->GetHp() <= bossB_->GetMaxHp() / 2);

	// 「どちらかが死亡」または「どちらかのHPが半分以下」になった場合に合体発動
	if (isBossADead || isBossBDead || isBossAHalf || isBossBHalf) {

		// 基本は Boss A を survivor（生き残り・合体主体）とする
		Boss* survivor = bossA_;
		Boss* victim = bossB_;

		// Boss A が死亡、あるいは Boss A のHPが半分以下（かつ Bはまだ大丈夫）なら Boss B を主体にする
		if (isBossADead || (isBossAHalf && !isBossBHalf)) {
			survivor = bossB_;
			victim = bossA_;
		}

		if (survivor) {
			// 生き残った（ベースとなる）ボスの位置で合体処理を実行
			Vector2 combinePos = survivor->GetPosition();
			survivor->CombineTo(combinePos, bossTextures_.combined);
		}

		if (victim) {
			// 縫い合わせ・攻撃対象リストから吸収される側のボスを削除
			targets_.erase(std::remove(targets_.begin(), targets_.end(), victim), targets_.end());

			// メンバー変数のポインタを安全にクリア
			if (victim == bossA_) {
				bossA_ = nullptr;
			} else {
				bossB_ = nullptr;
			}

			delete victim;
		}

		// 第2フェーズフラグをON
		isBossPhase2_ = true;
	}
}
void GameScene::PushPlayerOutOfBosses() {
	AABB2 p = player_->GetAABB();
	Vector2 pos = player_->GetPosition();
	for (StitchTarget* target : targets_) {
		if (!target || target->IsDead())
			continue;
		if (target->GetKind() != StitchTarget::Kind::kBoss && target->GetKind() != StitchTarget::Kind::kFodder && target->GetKind() != StitchTarget::Kind::kFlyer && target->GetKind() != StitchTarget::Kind::kRanged && target->GetKind() != StitchTarget::Kind::kHeavy) {
			continue;
		}
		AABB2 b = target->GetAABB();
		if (!IsCollision(p, b))
			continue;
		const float pushRight = b.max.x - p.min.x;
		const float pushLeft = p.max.x - b.min.x;
		const float pushDown = b.max.y - p.min.y;
		const float pushUp = p.max.y - b.min.y;
		float dx = (pushRight < pushLeft) ? pushRight : -pushLeft;
		float dy = (pushDown < pushUp) ? pushDown : -pushUp;
		if (std::abs(dx) <= std::abs(dy)) {
			pos.x += dx + (dx >= 0.0f ? 2.0f : -2.0f);
		} else {
			pos.y += dy + (dy >= 0.0f ? 2.0f : -2.0f);
		}
		player_->SetPosition(pos);
		p = player_->GetAABB();
	}
}

void GameScene::Update() {
	if (enterWait_ > 0) {
		--enterWait_;
		UpdateCamera();
		return;
	}

	player_->Update(mapChipField_);
	TryEnterNextArea();

	// ボスの更新（プレイヤーの参照を渡す場合）
	if (bossA_)
		bossA_->Update(mapChipField_, player_);
	if (bossB_)
		bossB_->Update(mapChipField_, player_);

	// 片方が倒れた際の合体フェーズ切り替え処理
	UpdateBossPhase();

	if (stakeL_)
		stakeL_->Update();
	if (stakeR_)
		stakeR_->Update();

	for (Enemy* enemy : fodder_) {
		if (enemy)
			enemy->Update(mapChipField_, player_);
	}
	for (Flyer* flyer : flyers_) {
		if (flyer)
			flyer->Update(mapChipField_, player_);
	}
	for (RangedEnemy* ranged : rangedEnemies_) {
		if (ranged)
			ranged->Update(mapChipField_, player_);
	}
	for (HeavyEnemy* heavy : heavyEnemies_) {
		if (heavy)
			heavy->Update(mapChipField_, player_);
	}

	// 敵同士の押し返し・反転処理
	Enemy::CheckEnemyCollisions(fodder_);

	CheckStitchOverlaps();
	hookStitch_->Update(player_, targets_);

	if (player_->InvincibleJustEnded()) {
		PushPlayerOutOfBosses();
	}

	CheckPlayerHits();
	UpdateCamera();

	if (player_->IsDead()) {
		isGameOver_ = true;
	}
}

void GameScene::DrawBlocks() {
	const Vector2 cam = camera_.GetOffset();

	for (const auto& block : blockObjects_) {
		if (!block.sprite)
			continue;

		block.sprite->SetRotation(0.0f);

		// カメラオフセットを適用して描画位置を調整
		block.sprite->SetPosition({block.position.x - cam.x, block.position.y - cam.y});
		block.sprite->SetSize({MapChipField::kTileWidth, MapChipField::kTileHeight});

		/*
		   ※もし 1 の通常ブロック（白テクスチャ）だけ色を付けたい場合は
		   必要に応じて SetColor を設定してください。
		   画像テクスチャ(10~18)をそのまま描画する場合はデフォルトの色(1,1,1,1)でOKです。
		*/
		// block.sprite->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

		block.sprite->Draw();
	}
}

void GameScene::DrawDoors() {
	const Vector2 cam = camera_.GetOffset();
	const bool opened = IsStageCleared();

	for (size_t i = 0; i < world_.doors.size() && i < doorSprites_.size(); ++i) {
		const DoorDesc& door = world_.doors[i];
		Sprite* sprite = doorSprites_[i];
		if (!sprite)
			continue;
		sprite->SetRotation(0.0f);
		sprite->SetColor(opened ? Vector4{0.25f, 0.55f, 0.3f, 0.45f} : Vector4{0.55f, 0.35f, 0.2f, 1.0f});
		sprite->SetPosition({door.x - cam.x, door.y - cam.y});
		sprite->SetSize({door.width, door.height});
		sprite->Draw();
	}
}

void GameScene::DrawHp() {
	const Vector2 cam = camera_.GetOffset();
	if (showPlayerHp_) {
		for (int i = 0; i < player_->GetMaxHp(); ++i) {
			Sprite* sprite = playerHpSprites_[i];
			if (!sprite) {
				continue;
			}
			sprite->SetRotation(0.0f);
			sprite->SetPosition({24.0f + static_cast<float>(i) * 36.0f, 20.0f});
			sprite->SetSize({28.0f, 28.0f});
			sprite->SetColor(i < player_->GetHp() ? Vector4{0.35f, 0.85f, 1.0f, 1.0f} : Vector4{0.2f, 0.25f, 0.3f, 1.0f});
			sprite->Draw();
		}
	}

	auto drawBossBar = [&](Boss* boss, Sprite* back, Sprite* fill) {
		if (!boss || boss->IsDead() || !back || !fill)
			return;
		const float barW = boss->GetSize().x;
		const float x = boss->GetPosition().x - cam.x;
		const float y = boss->GetPosition().y - 18.0f - cam.y;
		const float ratio = static_cast<float>(boss->GetHp()) / static_cast<float>(boss->GetMaxHp());
		back->SetRotation(0.0f);
		back->SetColor({0.1f, 0.1f, 0.1f, 0.9f});
		back->SetPosition({x, y});
		back->SetSize({barW, 10.0f});
		back->Draw();
		fill->SetRotation(0.0f);
		fill->SetColor({0.9f, 0.2f, 0.2f, 1.0f});
		fill->SetPosition({x, y});
		fill->SetSize({barW * ratio, 10.0f});
		fill->Draw();
	};

	// ボスが存在していれば無条件でHPバーを描画
	if (bossA_) {
		drawBossBar(bossA_, bossAHpBack_, bossAHpFill_);
	}
	if (bossB_) {
		drawBossBar(bossB_, bossBHpBack_, bossBHpFill_);
	}

	for (size_t i = 0; i < fodder_.size() && i < static_cast<size_t>(kMaxFodderUi); ++i) {
		Enemy* enemy = fodder_[i];
		if (!enemy || enemy->IsDead())
			continue;
		const float barW = enemy->GetSize().x;
		const float x = enemy->GetPosition().x - cam.x;
		const float y = enemy->GetPosition().y - 14.0f - cam.y;
		const float ratio = static_cast<float>(enemy->GetHp()) / static_cast<float>(enemy->GetMaxHp());
		fodderHpBack_[i]->SetRotation(0.0f);
		fodderHpBack_[i]->SetColor({0.1f, 0.1f, 0.1f, 0.9f});
		fodderHpBack_[i]->SetPosition({x, y});
		fodderHpBack_[i]->SetSize({barW, 8.0f});
		fodderHpBack_[i]->Draw();
		fodderHpFill_[i]->SetRotation(0.0f);
		fodderHpFill_[i]->SetColor({0.35f, 0.9f, 0.35f, 1.0f});
		fodderHpFill_[i]->SetPosition({x, y});
		fodderHpFill_[i]->SetSize({barW * ratio, 8.0f});
		fodderHpFill_[i]->Draw();
	}

	for (size_t i = 0; i < flyers_.size() && i < static_cast<size_t>(kMaxFlyerUi); ++i) {
		Flyer* flyer = flyers_[i];
		if (!flyer || flyer->IsDead())
			continue;
		const float barW = flyer->GetSize().x;
		const float x = flyer->GetPosition().x - cam.x;
		const float y = flyer->GetPosition().y - 14.0f - cam.y;
		const float ratio = static_cast<float>(flyer->GetHp()) / static_cast<float>(flyer->GetMaxHp());
		flyerHpBack_[i]->SetRotation(0.0f);
		flyerHpBack_[i]->SetColor({0.1f, 0.1f, 0.1f, 0.9f});
		flyerHpBack_[i]->SetPosition({x, y});
		flyerHpBack_[i]->SetSize({barW, 8.0f});
		flyerHpBack_[i]->Draw();
		flyerHpFill_[i]->SetRotation(0.0f);
		flyerHpFill_[i]->SetColor({0.85f, 0.35f, 0.95f, 1.0f});
		flyerHpFill_[i]->SetPosition({x, y});
		flyerHpFill_[i]->SetSize({barW * ratio, 8.0f});
		flyerHpFill_[i]->Draw();
	}
	// 遠距離敵のHPバー描画
	for (size_t i = 0; i < rangedEnemies_.size() && i < static_cast<size_t>(kMaxRangedUi); ++i) {
		RangedEnemy* ranged = rangedEnemies_[i];
		if (!ranged || ranged->IsDead())
			continue;

		const float barW = ranged->GetSize().x;
		const float x = ranged->GetPosition().x - cam.x;
		const float y = ranged->GetPosition().y - 14.0f - cam.y;
		const float ratio = static_cast<float>(ranged->GetHp()) / static_cast<float>(ranged->GetMaxHp());

		rangedHpBack_[i]->SetRotation(0.0f);
		rangedHpBack_[i]->SetColor({0.1f, 0.1f, 0.1f, 0.9f});
		rangedHpBack_[i]->SetPosition({x, y});
		rangedHpBack_[i]->SetSize({barW, 8.0f});
		rangedHpBack_[i]->Draw();

		rangedHpFill_[i]->SetRotation(0.0f);
		rangedHpFill_[i]->SetColor({0.7f, 0.3f, 0.8f, 1.0f}); // 紫色
		rangedHpFill_[i]->SetPosition({x, y});
		rangedHpFill_[i]->SetSize({barW * ratio, 8.0f});
		rangedHpFill_[i]->Draw();
	}
	for (size_t i = 0; i < heavyEnemies_.size() && i < static_cast<size_t>(kMaxHeavyUi); ++i) {
		HeavyEnemy* heavy = heavyEnemies_[i];
		if (!heavy || heavy->IsDead())
			continue;
		const float barW = heavy->GetSize().x;
		const float x = heavy->GetPosition().x - cam.x;
		const float y = heavy->GetPosition().y - 14.0f - cam.y;
		const float ratio = static_cast<float>(heavy->GetHp()) / static_cast<float>(heavy->GetMaxHp());
		heavyHpBack_[i]->SetRotation(0.0f);
		heavyHpBack_[i]->SetColor({0.1f, 0.1f, 0.1f, 0.9f});
		heavyHpBack_[i]->SetPosition({x, y});
		heavyHpBack_[i]->SetSize({barW, 8.0f});
		heavyHpBack_[i]->Draw();
		heavyHpFill_[i]->SetRotation(0.0f);
		heavyHpFill_[i]->SetColor({0.9f, 0.6f, 0.2f, 1.0f}); // オレンジ色
		heavyHpFill_[i]->SetPosition({x, y});
		heavyHpFill_[i]->SetSize({barW * ratio, 8.0f});
		heavyHpFill_[i]->Draw();
	}
}

void GameScene::Draw() {
	const Vector2 cam = camera_.GetOffset();

	Sprite::PreDraw();
	if (bgLeftSprite_) {
		bgLeftSprite_->SetPosition({0.0f - cam.x, 0.0f - cam.y});
		bgLeftSprite_->Draw();
	}
	if (bgRightSprite_) {
		bgRightSprite_->SetPosition({1280.0f - cam.x, 0.0f - cam.y});
		bgRightSprite_->Draw();
	}
	DrawBlocks();
	DrawDoors();
	if (stakeL_)
		stakeL_->Draw(cam);
	if (stakeR_)
		stakeR_->Draw(cam);

	if (bossA_) {
		bossA_->Draw(cam);
		bossA_->DrawDebugFrame(cam); // ★ デバッグ枠描画
	}
	if (bossB_) {
		bossB_->Draw(cam);
		bossB_->DrawDebugFrame(cam);
	}
	for (Enemy* enemy : fodder_) {
		if (enemy)
			enemy->Draw(cam);
	}
	for (Flyer* flyer : flyers_) {
		if (flyer)
			flyer->Draw(cam);
	}
	for (RangedEnemy* ranged : rangedEnemies_) {
		if (ranged)
			ranged->Draw(cam);
	}
	for (HeavyEnemy* heavy : heavyEnemies_) {
		if (heavy)
			heavy->Draw(cam);
	}
	player_->Draw(cam);
	hookStitch_->Draw(player_, cam);
	DrawHp();
	Sprite::PostDraw();
}