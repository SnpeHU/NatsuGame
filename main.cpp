#include <Windows.h>
#include "KamataEngine.h"
#include "SceneManager.h"

using namespace KamataEngine;
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	
	KamataEngine::Initialize(L"GC2A_04_コウ_ホウケイ_AL3");
	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();

	// シーンマネージャーの初期化
	SceneManager& sceneManager = SceneManager::GetInstance();
	sceneManager.Init();


	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

		// ImGui受付開始
		imguiManager->Begin();

		sceneManager.Update();

		// ImGui受付終了
		imguiManager->End();

		// 描画開始
		dxCommon_->PreDraw();
		// シーンマネージャーの描画
		sceneManager.Draw();
		
		//  AxisIndicatorの描画
		AxisIndicator::GetInstance()->Draw();

		// ImGui描画
		imguiManager->Draw();

		// 描画終了
		dxCommon_->PostDraw();
	}

	// シーンマネージャーの終了処理 - 智能指针会自动清理
	SceneManager::Destroy();

	KamataEngine::Finalize();
	return 0;
}
