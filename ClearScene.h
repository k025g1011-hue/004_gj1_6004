#pragma once

#include "BaseScene.h"
#include "KamataEngine.h"

using namespace KamataEngine;

/// <summary>
/// クリア（リザルト）シーン
/// </summary>
class ClearScene : public BaseScene {
public:
	~ClearScene() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;

private:
	// 背景画像用
	uint32_t bgTextureHandle_ = 0u;
	KamataEngine::Sprite* bgSprite_ = nullptr;

	// シーン開始直後の誤入力を防ぐタイマー
	int inputWaitTimer_ = 0;
};