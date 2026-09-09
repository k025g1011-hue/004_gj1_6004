#pragma once
#include "GameScene.h"
#include <array>

class TutorialScene : public GameScene {
public:
	~TutorialScene() override;
	void Initialize() override;
	void Update() override;
	void Draw() override;

private:
	enum class Hint {
		kMove = 0,
		kJump,
		kDashStitch,
		kDashStitch2,
		kCinch,
		kDoor,
		kCount,
	};

	void UpdateHint();
	void UpdateHintSprite();

	Hint hint_ = Hint::kMove;
	bool wasOnGround_ = false;
	bool hasLanded_ = false;
	bool sawCinch_ = false;

	std::array<uint32_t, static_cast<size_t>(Hint::kCount)> hintTextures_{};
	KamataEngine::Sprite* hintSprite_ = nullptr;

	static inline const float kHintW = 420.0f;
	static inline const float kHintH = 130.0f;
	static inline const float kHintX = 1280.0f - kHintW;
	static inline const float kHintY = 0.0f;
};