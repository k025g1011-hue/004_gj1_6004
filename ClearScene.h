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
	// 背景スプライト用
	uint32_t textureHandleBG_ = 0u;
	KamataEngine::Sprite* backgroundSprite_ = nullptr;

	// 操作案内UIスプライト
	uint32_t textureHandleSpace_ = 0u;
	KamataEngine::Sprite* spaceSprite_ = nullptr;

	// シーン開始直後の誤入力を防ぐタイマー
	int inputWaitTimer_ = 0;
};