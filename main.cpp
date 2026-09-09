#include "KamataEngine.h"
#include "SceneManager.h"
#include <Windows.h>

using namespace KamataEngine;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	KamataEngine::Initialize(L" 004_gj1_6004_ぬいぐるみなんてだいっきらい");
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

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
	KamataEngine::Finalize();
	return 0;
}