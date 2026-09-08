#pragma once
#include "BaseScene.h" // ★ BaseSceneをインクルード
#include "Fade.h"
#include "KamataEngine.h"

class TutorialScene : public BaseScene { // ★ BaseSceneを継承
public:
	enum class Phase { kFadeIn, kMain, kFadeOut };

	TutorialScene() = default;
	~TutorialScene() override; // ★ override を指定

	void Initialize() override;
	void Update() override;
	void Draw() override;

	// ★ IsFinished() や isFinished_ は BaseScene のものをそのまま使うため削除

private:
	// チュートリアル一枚絵用
	uint32_t textureHandle_ = 0u;
	KamataEngine::Sprite* sprite_ = nullptr;

	// フェード制御
	Fade* fade_ = nullptr;
	Phase phase_ = Phase::kFadeIn;
};