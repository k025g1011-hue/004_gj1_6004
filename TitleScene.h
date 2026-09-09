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

	// テクスチャハンドル
	uint32_t bgTextureHandle_ = 0u;
	uint32_t textureHandleSpace_ = 0u;

	// スプライト
	KamataEngine::Sprite* bgSprite_ = nullptr;
	KamataEngine::Sprite* spaceSprite_ = nullptr;

	// 点滅処理用
	float blinkTimer_ = 0.0f;
	bool isSpaceVisible_ = true;
};