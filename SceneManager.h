#pragma once
#include "BaseScene.h"
#include "Fade.h"

// シーンの種類
enum class SceneType { kTitle, kTutorial, kStage1, kStage2, kStage3, kBoss, kClear, kGameOver };

class SceneManager {
public:
	void Initialize();
	void Update();
	void Draw();

	// 外部からシーン切り替えを呼び出したい場合（必要に応じて）
	void ChangeScene(SceneType nextScene);

private:
	// シーン生成ヘルパー
	BaseScene* CreateScene(SceneType type);

private:
	BaseScene* currentScene_ = nullptr;
	SceneType currentType_ = SceneType::kTitle;
	SceneType nextType_ = SceneType::kTitle;
	SceneType retryType_ = SceneType::kStage1;

	Fade fade_;

	enum class State {
		kNormal,  // 通常プレイ中
		kFadeOut, // 暗転中
		kFadeIn   // 明転中
	};
	State state_ = State::kNormal;

	int playerHp_ = 3;
};