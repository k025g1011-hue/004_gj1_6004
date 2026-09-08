#pragma once
#include "KamataEngine.h"

/// <summary>
/// 2Dカメラ。今は部屋ごとの固定座標だけ使う。
/// position_ は画面左上に対応するワールド座標。
/// 描画時は world - position_ でスクリーン座標にする。
/// </summary>
class Camera2D {
public:
	void SetFixed(const KamataEngine::Vector2& topLeft);
	void UpdateFollow(const KamataEngine::Vector2& playerCenter, float mapLeft, float mapRight, float mapTop, float mapBottom);

	const KamataEngine::Vector2& GetOffset() const { return position_; }

	static inline const float kViewW = 1280.0f;
	static inline const float kViewH = 720.0f;

private:
	KamataEngine::Vector2 position_{};
	static inline const float kDeadZoneY = 48.0f;
};