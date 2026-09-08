#include "SceneManager.h"
#include "BossScene.h"
#include "ClearScene.h"
#include "Stage1Scene.h"
#include "Stage2Scene.h"
#include "Stage3Scene.h"
#include "TitleScene.h"
#include "TutorialScene.h"
#include "GameOverScene.h"

void SceneManager::Initialize() {
	fade_.Initialize();
	currentType_ = SceneType::kTitle;
	currentScene_ = CreateScene(currentType_);
	currentScene_->Initialize();
}

BaseScene* SceneManager::CreateScene(SceneType type) {
	switch (type) {
	case SceneType::kTitle:
		return new TitleScene();
	case SceneType::kTutorial:
		return new TutorialScene();
	case SceneType::kStage1:
		return new Stage1Scene();
	case SceneType::kStage2:
		return new Stage2Scene();
	case SceneType::kStage3:
		return new Stage3Scene();
	case SceneType::kBoss:
		return new BossScene();
	case SceneType::kClear:
		return new ClearScene();
	case SceneType::kGameOver:
		return new GameOverScene();
	}
	return nullptr;
}

void SceneManager::Update() {
	fade_.Update();

	switch (state_) {
	case State::kNormal:
		if (currentScene_) {
			currentScene_->Update();
			// デバッグログ出力（IsGameOver も確認できるように追加）
			char buf[256];
			sprintf_s(
			    buf, "[SceneManager] SceneType:%d | Finished:%s | GameOver:%s\n", static_cast<int>(currentType_), currentScene_->IsFinished() ? "TRUE" : "FALSE",
			    currentScene_->IsGameOver() ? "TRUE" : "FALSE");
			OutputDebugStringA(buf);

			// ★ 1. プレイヤーが死亡（GameOver）した場合
			if (currentScene_->IsGameOver()) {
				retryType_ = currentType_;        // 死んだステージを記録
				nextType_ = SceneType::kGameOver; // 次は GameOver 画面へ
				playerHp_ = 3;

				state_ = State::kFadeOut;
				fade_.Start(Status::FadeOut, 0.5f);
			}
			// ★ 2. ステージクリア または GameOver画面でスペースを押した場合
			else if (currentScene_->IsFinished()) {
				// 次の行き先を決めるルーティング
				if (currentType_ == SceneType::kTitle)
					nextType_ = SceneType::kTutorial;
				else if (currentType_ == SceneType::kTutorial)
					nextType_ = SceneType::kStage1;
				else if (currentType_ == SceneType::kStage1)
					nextType_ = SceneType::kStage2;
				else if (currentType_ == SceneType::kStage2)
					nextType_ = SceneType::kStage3;
				else if (currentType_ == SceneType::kStage3)
					nextType_ = SceneType::kBoss;
				else if (currentType_ == SceneType::kBoss)
					nextType_ = SceneType::kClear;
				else if (currentType_ == SceneType::kClear)
					nextType_ = SceneType::kTitle;
				else if (currentType_ == SceneType::kGameOver) 
					nextType_ = retryType_;                    

				state_ = State::kFadeOut;
				fade_.Start(Status::FadeOut, 0.5f);
			}
		}
		break;

	case State::kFadeOut:
		// 暗転が完了したらシーンを差し替える
		if (fade_.IsFinished()) {
			delete currentScene_;
			currentType_ = nextType_;

			currentScene_ = CreateScene(currentType_);
			currentScene_->Initialize();

			state_ = State::kFadeIn;
			fade_.Start(Status::FadeIn, 0.5f);
		}
		break;

	case State::kFadeIn:
		// 明転中の更新停止
		if (fade_.IsFinished()) {
			fade_.Stop();
			state_ = State::kNormal;
		}
		break;
	}
}

void SceneManager::Draw() {
	if (currentScene_) {
		currentScene_->Draw();
	}
	// 最前面にFadeを描画
	fade_.Draw();
}