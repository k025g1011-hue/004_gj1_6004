// BaseScene.h
#pragma once
#include "KamataEngine.h"
#include "Player.h"

class BaseScene {
public:
	virtual ~BaseScene() = default;

	// 純粋仮想関数（派生クラスで必ず実装する）
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	// シーンの終了フラグ取得
	virtual bool IsFinished() const { return isFinished_; }
	virtual bool IsGameOver() const { return isGameOver_; }

protected:
	bool isFinished_ = false; // クリアや画面遷移のトリガー
	bool isGameOver_ = false;
};