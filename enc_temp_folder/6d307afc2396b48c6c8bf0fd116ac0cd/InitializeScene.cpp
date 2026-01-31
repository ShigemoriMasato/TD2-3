#include "../InitializeScene.h"
#include <Test/TestScene.h>
#include <Game/GameScene.h>
#include <ModelEditScene/ModelEditScene.h>
#include<Game/SelectScene.h>
#include <imgui/imgui.h>
#include <Utility/DataStructures.h>
#include<Common/DebugParam/GameParamEditor.h>

#ifdef USE_IMGUI
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

void InitializeScene::Initialize() {

	CreateDisplay();
	EngineSetup();
	SetupKeyManager();

	//OffScreen用の頂点データ登録
	std::vector<VertexData> vertices = {
		{{-1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
		{{3.0f, 1.0f, 0.0f, 1.0f}, {2.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
		{{-1.0f, -3.0f, 0.0f, 1.0f}, {0.0f, 2.0f}, {0.0f, 0.0f, -1.0f}}
	};
	drawDataManager_->AddVertexBuffer(vertices);
	commonData_->postEffectDrawDataIndex = drawDataManager_->CreateDrawData();

	commonData_->newMapManager = std::make_unique<NewMapManager>();
	commonData_->newMapManager->Initialize(modelManager_);

	// 保存されているパラメーターを取得する
	GameParamEditor::GetInstance()->LoadFiles();

	// テクスチャを全てロード
	textureManager_->LoadAllTextures();

	commonData_->nextStageIndex = 0;
	commonData_->nextMapIndex = 1;
}

std::unique_ptr<IScene> InitializeScene::Update() {
	return std::make_unique<ModelEditScene>();
<<<<<<< HEAD
	//return std::make_unique<GameScene>();
=======
	return std::make_unique<GameScene>();
>>>>>>> cbf7fa3842ad51b4ed1a18908125c3536afb03e7
}

void InitializeScene::Draw() {
	commonData_->display->PreDraw(commonData_->mainWindow->GetCommandObject(), true);
	commonData_->display->PostDraw(commonData_->mainWindow->GetCommandObject());
	commonData_->mainWindow->PreDraw(true);
	commonData_->mainWindow->DrawDisplayWithImGui();
}

void InitializeScene::CreateDisplay() {
	{
		WindowConfig config;
		config.windowName = L"2208_Ore";
		CommonData* cd = commonData_;
		config.windowProc = [cd](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)->LRESULT {

#ifdef USE_IMGUI
			if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
				return true;
			}
#endif

			switch (msg) {
			case WM_DESTROY:
				PostQuitMessage(0);
				//WindowsProcが通っているかの確認用
				cd->exeFinished_ = true;
				return 0;
			}
			return DefWindowProc(hwnd, msg, wparam, lparam);
			};

		config.clientWidth = 1280;
		config.clientHeight = 720;

		uint32_t clearColor = 0x0060b0ff;

		commonData_->mainWindow = std::make_unique<GameWindow>();
		commonData_->mainWindow->Initialize(engine_, config, clearColor);
	}

	engine_->ImGuiActivate(commonData_->mainWindow->GetWindow());

	{
		auto& display = commonData_->display;
		display = std::make_unique<DualDisplay>("MainDisplay");

		uint32_t clearColor = 0x000105ff;

		int textureIndex = textureManager_->CreateWindowTexture(1280, 720, clearColor);
		int textureIndex2 = textureManager_->CreateWindowTexture(1280, 720, clearColor);
		auto textureData = textureManager_->GetTextureData(textureIndex);
		auto textureData2 = textureManager_->GetTextureData(textureIndex2);
		display->Initialize(textureData, textureData2);

		commonData_->mainWindow->AddDisplay(textureIndex, "Main Display", 1280 / 2, 720 / 2);
	}

}

void InitializeScene::EngineSetup() {
	//Input
	input_->SetHWND(commonData_->mainWindow->GetWindow()->GetHwnd());

	//FPS
	auto fps = engine_->GetFPSObserver();
	fps->SetIsFix(true);
	fps->SetTargetFPS(60.0);
}

void InitializeScene::SetupKeyManager() {
	commonData_->keyManager = std::make_unique<KeyManager>();

	auto keyManager = commonData_->keyManager.get();
	keyManager->Initialize();

	//KeyConfigの設定
	keyManager->SetKey(Key::Right, DIK_D, KeyState::Hold);
	keyManager->SetKey(Key::Right, DIK_RIGHTARROW, KeyState::Hold);
	keyManager->SetButton(Key::Right, XBoxController::kRight, KeyState::Hold);
	keyManager->SetStick(Key::Right, true, false, 0.5f);

	keyManager->SetKey(Key::Left, DIK_A, KeyState::Hold);
	keyManager->SetKey(Key::Left, DIK_LEFTARROW, KeyState::Hold);
	keyManager->SetButton(Key::Left, XBoxController::kLeft, KeyState::Hold);
	keyManager->SetStick(Key::Left, true, false, -0.5f);

	keyManager->SetKey(Key::Up, DIK_W, KeyState::Hold);
	keyManager->SetKey(Key::Up, DIK_UPARROW, KeyState::Hold);
	keyManager->SetButton(Key::Up, XBoxController::kUp, KeyState::Hold);
	keyManager->SetStick(Key::Up, true, true, 0.5f);

	keyManager->SetKey(Key::Down, DIK_S, KeyState::Hold);
	keyManager->SetKey(Key::Down, DIK_DOWNARROW, KeyState::Hold);
	keyManager->SetButton(Key::Down, XBoxController::kDown, KeyState::Hold);
	keyManager->SetStick(Key::Down, true, true, -0.5f);

	// トリガー
	keyManager->SetKey(Key::RightTri, DIK_D, KeyState::Trigger);
	keyManager->SetKey(Key::RightTri, DIK_RIGHTARROW, KeyState::Trigger);
	keyManager->SetButton(Key::RightTri, XBoxController::kRight, KeyState::Trigger);
	keyManager->SetStick(Key::RightTri, true, false, 0.5f);

	keyManager->SetKey(Key::LeftTri, DIK_A, KeyState::Trigger);
	keyManager->SetKey(Key::LeftTri, DIK_LEFTARROW, KeyState::Trigger);
	keyManager->SetButton(Key::LeftTri, XBoxController::kLeft, KeyState::Trigger);
	keyManager->SetStick(Key::LeftTri, true, false, -0.5f);

	keyManager->SetKey(Key::UpTri, DIK_W, KeyState::Trigger);
	keyManager->SetKey(Key::UpTri, DIK_UPARROW, KeyState::Trigger);
	keyManager->SetButton(Key::UpTri, XBoxController::kUp, KeyState::Trigger);
	keyManager->SetStick(Key::UpTri, true, true, 0.5f);

	keyManager->SetKey(Key::DownTri, DIK_S, KeyState::Trigger);
	keyManager->SetKey(Key::DownTri, DIK_DOWNARROW, KeyState::Trigger);
	keyManager->SetButton(Key::DownTri, XBoxController::kDown, KeyState::Trigger);
	keyManager->SetStick(Key::DownTri, true, true, -0.5f);

	keyManager->SetKey(Key::Decision, DIK_SPACE, KeyState::Trigger);

	//================================================================================

	keyManager->SetKey(Key::HardDrop, DIK_W, KeyState::Trigger);
	keyManager->SetKey(Key::HardDrop, DIK_SPACE, KeyState::Trigger);
	keyManager->SetKey(Key::HardDrop, DIK_UPARROW, KeyState::Trigger);
	keyManager->SetButton(Key::HardDrop, XBoxController::kUp, KeyState::Trigger);
	keyManager->SetStick(Key::HardDrop, true, true, 0.5f);

	keyManager->SetKey(Key::Hold, DIK_LSHIFT, KeyState::Trigger);
	keyManager->SetKey(Key::Hold, DIK_C, KeyState::Trigger);
	keyManager->SetKey(Key::Hold, DIK_H, KeyState::Trigger);
	keyManager->SetKey(Key::Hold, DIK_L, KeyState::Trigger);
	keyManager->SetKey(Key::Hold, DIK_RSHIFT, KeyState::Trigger);
	keyManager->SetButton(Key::Hold, XBoxController::kLeftShoulder, KeyState::Trigger);
	keyManager->SetButton(Key::Hold, XBoxController::kLeftTrigger, KeyState::Trigger);
	keyManager->SetButton(Key::Hold, XBoxController::kRightShoulder, KeyState::Trigger);
	keyManager->SetButton(Key::Hold, XBoxController::kLeftTrigger, KeyState::Trigger);

	//================================================================================

	keyManager->SetKey(Key::LRotate, DIK_Z, KeyState::Trigger);
	keyManager->SetKey(Key::LRotate, DIK_J, KeyState::Trigger);
	keyManager->SetButton(Key::LRotate, XBoxController::kX, KeyState::Trigger);

	keyManager->SetKey(Key::RRotate, DIK_X, KeyState::Trigger);
	keyManager->SetKey(Key::RRotate, DIK_K, KeyState::Trigger);
	keyManager->SetButton(Key::RRotate, XBoxController::kY, KeyState::Trigger);

	//================================================================================

	keyManager->SetKey(Key::Correct, DIK_RETURN, KeyState::Trigger);
	keyManager->SetKey(Key::Correct, DIK_SPACE, KeyState::Trigger);
	keyManager->SetKey(Key::Correct, DIK_Z, KeyState::Trigger);
	keyManager->SetButton(Key::Correct, XBoxController::kA, KeyState::Trigger);

	keyManager->SetKey(Key::Reverse, DIK_X, KeyState::Trigger);
	keyManager->SetButton(Key::Reverse, XBoxController::kB, KeyState::Trigger);

	keyManager->SetKey(Key::Pause, DIK_ESCAPE, KeyState::Trigger);
	keyManager->SetKey(Key::Pause, DIK_F1, KeyState::Trigger);
	keyManager->SetButton(Key::Pause, XBoxController::kStart, KeyState::Trigger);

	//================================================================================

	keyManager->SetKey(Key::Restart, DIK_R, KeyState::Trigger);
	keyManager->SetKey(Key::Restart, DIK_ESCAPE, KeyState::Trigger);
	keyManager->SetButton(Key::Restart, XBoxController::kSelect, KeyState::Trigger);

	keyManager->SetKey(Key::Debug1, DIK_F1, KeyState::Trigger);
	keyManager->SetKey(Key::Debug2, DIK_F2, KeyState::Trigger);
	keyManager->SetKey(Key::Debug3, DIK_F3, KeyState::Trigger);
}
