#include "KamataEngine.h"
#include "SceneManager.h"
#include "SoundManager.h"
#include <Windows.h>

using namespace KamataEngine;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	KamataEngine::Initialize(L" 004_gj1_6004_ぬいぐるみなんてだいっきらい");
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();


	SoundManager* soundManager = SoundManager::GetInstance();

	soundManager->Initialize();

	// 通常BGM
	soundManager->LoadBGM("NormalBGM", "./Resources/sounds/BGM/normal_bgm.wav");

	// ボスBGM
	soundManager->LoadBGM("BossBGM", "./Resources/sounds/BGM/boss_bgm.wav");

	SoundManager::GetInstance()->LoadSE("Stitch", "./Resources/sounds/SE/Stitch.wav");
	SoundManager::GetInstance()->LoadSE("ArrowEnemy", "./Resources/sounds/SE/ArrowEnemy.wav");
	SoundManager::GetInstance()->LoadSE("Dash", "./Resources/sounds/SE/Dash.wav");
	SoundManager::GetInstance()->LoadSE("Jump", "./Resources/sounds/SE/Jump.wav");
	SoundManager::GetInstance()->LoadSE("Damage", "./Resources/sounds/SE/Damage.wav");
	SoundManager::GetInstance()->LoadSE("SceanTransition", "./Resources/sounds/SE/SceanTransition.wav");
	SoundManager::GetInstance()->LoadSE("Hit", "./Resources/sounds/SE/Hit.wav");

	// SceneManagerの生成と初期化
	SceneManager* sceneManager = new SceneManager();
	sceneManager->Initialize();
	// メインループ
	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

		// 更新
		sceneManager->Update();

		// 描画
		dxCommon->PreDraw();
		sceneManager->Draw();
		dxCommon->PostDraw();
	}

	// 解放
	delete sceneManager;
	// SoundManagerの終了処理
	soundManager->Finalize();
	KamataEngine::Finalize();
	return 0;
}