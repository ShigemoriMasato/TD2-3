#pragma once
#include <Core/DXDevice.h>
#include <Core/FenceManager.h>
#include <Core/ImGuiforEngine.h>
#include <Utility/DirectUtilFuncs.h>
#include <Screen/Window.h>
#include <Screen/WindowMaker.h>
#include <Render/DrawDataManager.h>
#include <Input/Input.h>
#include <Tool/FPS/FPSObserver.h>
#include <Assets/Model/ModelManager.h>
#include <memory>

class SHEngine {
public:

	SHEngine();
	~SHEngine();

	void Initialize(HINSTANCE hInstance);

	bool IsLoop();
	void Update();
	bool PreDraw();
	void EndFrame();

	void WaitForGPU();

	void ImGuiActivate(Window* window);
	void DrawImGui();

	TextureManager* GetTextureManager() { return textureManager_.get(); }
	ModelManager* GetModelManager() { return modelManager_.get(); }
	WindowMaker* GetWindowMaker() { return windowMaker_.get(); }
	DrawDataManager* GetDrawDataManager() { return drawDataManager_.get(); }
	Input* GetInput() { return input_.get(); }
	FPSObserver* GetFPSObserver() { return fpsObserver_.get(); }

private:

	void ExecuteMessage();
	void FPSDraw();

private:

	//LeakChecker
	D3DResourceLeakChecker leakChecker_{};

	//最後に削除する
	std::unique_ptr<DXDevice> dxDevice_ = nullptr;

private:

	//Engine側ツール
	std::unique_ptr<CmdListManager> cmdListManager_ = nullptr;
	std::unique_ptr<FenceManager> fenceManager_ = nullptr;
	std::unique_ptr<ImGuiforEngine> imGuiForEngine_ = nullptr;
	bool inputActive_ = false;

	//App側ツール
	std::unique_ptr<TextureManager> textureManager_ = nullptr;
	std::unique_ptr<ModelManager> modelManager_ = nullptr;
	std::unique_ptr<DrawDataManager> drawDataManager_ = nullptr;
	std::unique_ptr<WindowMaker> windowMaker_ = nullptr;
	std::unique_ptr<Input> input_ = nullptr;
	std::unique_ptr<FPSObserver> fpsObserver_ = nullptr;

private:

	bool exit_ = false;
	bool pushedCrossButton_ = false;

	bool imGuiActive_ = false;
	bool imguiDrawed_ = false;

	HINSTANCE hInstance_ = nullptr;
	MSG msg_{};

	Logger logger_ = nullptr;
};
