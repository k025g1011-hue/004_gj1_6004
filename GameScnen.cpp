#include "GameScene.h"
#include <cmath>

using namespace KamataEngine;

GameScene::~GameScene() {
	ClearCurrentActors();
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
	targets_.clear();
	world_.doors.clear();

	for (Sprite* sprite : blockSprites_) {
		delete sprite;
	}
	blockSprites_.clear();
	blockPositions_.clear();
}

void GameScene::Initialize() {
	isFinished_ = false;

	whiteTexture_ = TextureManager::Load("white.png");
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

	player_ = new Player();
	player_->Initialize(whiteTexture_);
	hookStitch_ = new HookStitch();
	hookStitch_->Initialize(whiteTexture_);

	mapChipField_ = new MapChipField();
}

void GameScene::BuildWorld(const std::string& csvPath) {
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

			if (type == MapChipType::kBlock) {
				blockSprites_.push_back(Sprite::Create(whiteTexture_, {0.0f, 0.0f}));
				blockPositions_.push_back(cell);
			} else if (type == MapChipType::kPlayer) {
				if (!foundPlayerSpawn) {
					playerSpawnPos = cell;
					playerSpawnPos.y = mapChipField_->SnapFeetToFloor(cell.x + 8.0f, 64.0f);
					foundPlayerSpawn = true;
				}
			} else if (type == MapChipType::kFodder) {
				EnemySpawn spawn;
				spawn.position = cell;
				spawn.position.y = mapChipField_->SnapFeetToFloor(cell.x + 10.0f, 80.0f);

				Enemy* enemy = new Enemy();
				// 位置情報のみを渡し、1280px画面基準の自動巡回エリアを適用
				enemy->Initialize(whiteTexture_, spawn.position);
				fodder_.push_back(enemy);
				fodderSpawns_.push_back(spawn);
				targets_.push_back(enemy);

			} else if (type == MapChipType::kFlyer) {
				EnemySpawn spawn;
				spawn.position = cell; // 空中の高さを維持するためスナップしない

				Flyer* flyer = new Flyer();
				flyer->Initialize(whiteTexture_, spawn.position);
				flyers_.push_back(flyer);
				flyerSpawns_.push_back(spawn);
				targets_.push_back(flyer);
			}else if (type == MapChipType::kDoor) {
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
				spawn.y = mapChipField_->SnapFeetToFloor(cell.x + 8.0f, 120.0f);
				Boss* boss = new Boss();
				const Vector4 color = (bossCount == 0) ? Vector4{0.95f, 0.25f, 0.25f, 1.0f} : Vector4{0.95f, 0.55f, 0.2f, 1.0f};
				boss->Initialize(whiteTexture_, spawn, color, spawn.x - 160.0f, spawn.x + 160.0f);
				if (bossCount == 0) {
					bossA_ = boss;
				} else {
					bossB_ = boss;
				}
				++bossCount;
				targets_.push_back(boss);
			}
		}
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
	return true;
}

void GameScene::TryEnterNextArea() {
	if (enterWait_ > 0)
		return;

	// 雑魚敵が全滅していなければドアに入れない
	if (!IsStageCleared())
		return;

	const Vector2 center = player_->GetCenter();
	for (const DoorDesc& door : world_.doors) {
		const float doorCenterX = door.x + door.width * 0.5f;
		const float doorCenterY = door.y + door.height * 0.5f;
		const float hitW = door.width * 0.25f;
		const float hitH = door.height * 0.25f;

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
		if (!target || !target->CanStitch() || target->IsDead())
			continue;
		if (IsCollision(playerAABB, target->GetAABB())) {
			hookStitch_->TryStitch(target);
		}
	}
}

void GameScene::CheckPlayerHits() {
	if (!player_ || player_->IsDead() || player_->IsInvincible() || player_->IsDashing() || hookStitch_->IsCinching()) {
		return;
	}
	AABB2 playerAABB = player_->GetAABB();
	for (StitchTarget* target : targets_) {
		if (!target || target->IsDead())
			continue;
		if (target->GetKind() != StitchTarget::Kind::kBoss && target->GetKind() != StitchTarget::Kind::kFodder && target->GetKind() != StitchTarget::Kind::kFlyer) {
			continue;
		}
		if (IsCollision(playerAABB, target->GetAABB())) {
			player_->OnDamaged();
			hookStitch_->Clear();
			break;
		}
	}
}

void GameScene::PushPlayerOutOfBosses() {
	AABB2 p = player_->GetAABB();
	Vector2 pos = player_->GetPosition();
	for (StitchTarget* target : targets_) {
		if (!target || target->IsDead())
			continue;
		if (target->GetKind() != StitchTarget::Kind::kBoss && target->GetKind() != StitchTarget::Kind::kFodder && target->GetKind() != StitchTarget::Kind::kFlyer) {
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

	if (bossA_)
		bossA_->Update(mapChipField_);
	if (bossB_)
		bossB_->Update(mapChipField_);
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
	for (size_t i = 0; i < blockSprites_.size(); ++i) {
		Sprite* sprite = blockSprites_[i];
		if (!sprite)
			continue;
		sprite->SetRotation(0.0f);
		sprite->SetColor({0.22f, 0.24f, 0.28f, 1.0f});
		sprite->SetPosition({blockPositions_[i].x - cam.x, blockPositions_[i].y - cam.y});
		sprite->SetSize({MapChipField::kTileWidth, MapChipField::kTileHeight});
		sprite->Draw();
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
	for (int i = 0; i < player_->GetMaxHp(); ++i) {
		Sprite* sprite = playerHpSprites_[i];
		if (!sprite)
			continue;
		sprite->SetRotation(0.0f);
		sprite->SetPosition({24.0f + static_cast<float>(i) * 36.0f, 20.0f});
		sprite->SetSize({28.0f, 28.0f});
		sprite->SetColor(i < player_->GetHp() ? Vector4{0.35f, 0.85f, 1.0f, 1.0f} : Vector4{0.2f, 0.25f, 0.3f, 1.0f});
		sprite->Draw();
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
}

void GameScene::Draw() {
	const Vector2 cam = camera_.GetOffset();

	Sprite::PreDraw();
	if (backSprite_) {
		backSprite_->SetColor({0.08f, 0.09f, 0.12f, 1.0f});
		backSprite_->SetPosition({0.0f, 0.0f});
		backSprite_->SetSize({1280.0f, 720.0f});
		backSprite_->Draw();
	}
	DrawBlocks();
	DrawDoors();
	if (stakeL_)
		stakeL_->Draw(cam);
	if (stakeR_)
		stakeR_->Draw(cam);
	if (bossA_)
		bossA_->Draw(cam);
	if (bossB_)
		bossB_->Draw(cam);
	for (Enemy* enemy : fodder_) {
		if (enemy)
			enemy->Draw(cam);
	}
	for (Flyer* flyer : flyers_) {
		if (flyer)
			flyer->Draw(cam);
	}
	player_->Draw(cam);
	hookStitch_->Draw(player_, cam);
	DrawHp();
	Sprite::PostDraw();
}