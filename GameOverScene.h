#pragma once

#include "BaseScene.h"
#include <kamataengine.h>

class GameOverScene : public BaseScene {
public:
	~GameOverScene() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;

private:
	uint32_t bgTextureHandle_ = 0;
	KamataEngine::Sprite* bgSprite_ = nullptr;
};