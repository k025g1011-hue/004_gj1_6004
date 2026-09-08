#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

const int kWindowWidth = 1280;
const int kWindowHeight = 720;

// フェードの状態
enum class Status {
	None,    // フェードなし
	FadeIn,  // フェードイン中
	FadeOut, // フェードアウト中
};

/// <summary>
/// フェード
/// </summary>
class Fade {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	void Start(Status status, float duration);

	void Stop();

	bool IsFinished() const;

private:
	Sprite* sprite_ = nullptr;

	// 現在のフェードの状態
	Status status_ = Status::None;

	// フェードの持続時間
	float duration_ = 0.0f;

	// 経過時間カウンター
	float counter_ = 0.0f;
};
