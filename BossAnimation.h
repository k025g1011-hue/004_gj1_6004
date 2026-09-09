#pragma once

#include "KamataEngine.h"

// 左右画像ペア（L/R）
struct DirectionalTexture {
	uint32_t left = 0;
	uint32_t right = 0;
};

// 1体分のボス（または合体ボス）が使用する全アニメーション素材セット
struct BossTextureSet {
	DirectionalTexture walk;  // 歩き (4コマ: 800x300)
	DirectionalTexture punch; // パンチ (4コマ: 800x300)
	uint32_t press = 0;       // ジャンププレス (8コマ: 1600x300 - 左右共通の一方向のみ)
};

// ゲーム全体のボス用リソースまとめ
struct AllBossTextures {
	BossTextureSet bossA;       // 第1フェーズ：ボスA
	BossTextureSet bossB;       // 第1フェーズ：ボスB
	BossTextureSet combined;    // 第2フェーズ：合体ボス
	uint32_t buttonTexture = 0; // 第2フェーズ用：ボタン落下物テクスチャ (40x40)
};