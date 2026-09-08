#pragma once
#include "BaseScene.h"
#include "KamataEngine.h"

using namespace KamataEngine;

/// <summary>
/// タイトルシーン
/// </summary>
class TitleScene : public BaseScene { 
public:
	~TitleScene() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;

private:
	// カメラ
	Camera camera_;

	float moveTimer_ = 0.0f;
	Vector3 startPosition_;

	uint32_t textureHandleSpace_ = 0u;
	KamataEngine::Sprite* spaceSprite_ = nullptr;
	float blinkTimer_ = 0.0f;

	uint32_t bgTextureHandle_ = 0u;
	KamataEngine::Sprite* bgSprite_ = nullptr;
};