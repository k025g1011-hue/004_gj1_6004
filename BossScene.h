#pragma once
#include "GameScene.h"
#include "Boss.h"


/// <summary>
/// ボスステージ
/// </summary>
class BossScene : public GameScene {
public:
	~BossScene() override = default;
	void Initialize() override;
	void Update() override; 
	void Draw() override;
};