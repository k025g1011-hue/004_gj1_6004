#pragma once
#include "KamataEngine.h"

/// <summary>
/// 2Dの軸平行バウンディングボックス。
/// min = 左上、max = 右下。
/// </summary>
struct AABB2 {
	KamataEngine::Vector2 min{};
	KamataEngine::Vector2 max{};
};

/// <summary>
/// 2つのAABBが重なっているか。1軸でも離れていれば未衝突。
/// </summary>
inline bool IsCollision(const AABB2& a, const AABB2& b) {
	if (a.max.x < b.min.x) {
		return false;
	}
	if (a.min.x > b.max.x) {
		return false;
	}
	if (a.max.y < b.min.y) {
		return false;
	}
	if (a.min.y > b.max.y) {
		return false;
	}
	return true;
}

/// <summary>
/// AABBの中心座標。縫いマークや重心計算に使う。
/// </summary>
inline KamataEngine::Vector2 AABBCenter(const AABB2& a) { return {(a.min.x + a.max.x) * 0.5f, (a.min.y + a.max.y) * 0.5f}; }