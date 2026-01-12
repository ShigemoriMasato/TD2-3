#include "SHEngine.h"
#include <Tool/Dump/CreateDump.h>
#include <Screen/DualDisplay.h>
#include <Render/RenderObject.h>
#include <imgui/imgui.h>

#pragma comment(lib, "Dbghelp.lib")

static LONG WINAPI ClashHandler(EXCEPTION_POINTERS* pExceptionPointers) {
	CreateDump(pExceptionPointers);
	return EXCEPTION_EXECUTE_HANDLER;
}

SHEngine::SHEngine() {

	logger_ = getLogger("Engine");

	//Initializeしても変わらない処理

	SetUnhandledExceptionFilter(ClashHandler);

	dxDevice_ = std::make_unique<DXDevice>();
	dxDevice_->Initialize();

	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr) && "SHEngine::SHEngine: Failed to initialize COM library");

	cmdListManager_ = std::make_unique<CmdListManager>();
	cmdListManager_->Initialize(dxDevice_.get());

	textureManager_ = std::make_unique<TextureManager>();
	textureManager_->Initialize(dxDevice_.get(), cmdListManager_.get());

	Display::StaticInitialize(dxDevice_.get());
	DualDisplay::StaticInitialize(dxDevice_.get());
	RenderObject::StaticInitialize(dxDevice_.get());

	fenceManager_ = std::make_unique<FenceManager>();
	fenceManager_->Initialize(cmdListManager_->GetCommandQueue(), dxDevice_->GetDevice());

	windowMaker_ = std::make_unique<WindowMaker>();
	windowMaker_->Initialize(dxDevice_.get(), textureManager_.get(), cmdListManager_.get());

	imGuiForEngine_ = std::make_unique<ImGuiforEngine>();

	drawDataManager_ = std::make_unique<DrawDataManager>();

	input_ = std::make_unique<Input>();

	fpsObserver_ = std::make_unique<FPSObserver>();
	fpsObserver_->SetIsFix(false, FPSType::GPU);
	fpsObserver_->SetTargetFPS(60.0, FPSType::GPU);

	modelManager_ = std::make_unique<ModelManager>();
}

SHEngine::~SHEngine() {
	fenceManager_->Finalize();
	if (imGuiActive_) {
		imGuiForEngine_->Finalize();
	}
}

void SHEngine::Initialize(HINSTANCE hInstance) {
	//初期化処理
	drawDataManager_->Initialize(dxDevice_.get());
	hInstance_ = hInstance;
	input_->Initialize(hInstance_);
	modelManager_->Initialize(textureManager_.get(), drawDataManager_.get());
}

bool SHEngine::IsLoop() {
	fpsObserver_->TimeAdjustment();

	return !exit_;
}

void SHEngine::Update() {
}

bool SHEngine::PreDraw() {
	//Wait
	if (!fenceManager_->SignalChecker()) {
		return false;
	}

	//ImGui
	if (imGuiActive_) {
		if (imguiDrawed_) {
			imGuiForEngine_->BeginFrame();
			imguiDrawed_ = false;
		}
		//FPS描画
		FPSDraw();
	}

	fpsObserver_->TimeAdjustment(FPSType::GPU);
	
	cmdListManager_->Reset();
	auto cmdObj = cmdListManager_->CreateCommandObject();
	textureManager_->UploadTextures(cmdObj->GetCommandList());
	return true;
}

void SHEngine::EndFrame() {
	//PostDraw
	windowMaker_->AllPostDraw();
	//ImGui
	DrawImGui();
	//Execute
	cmdListManager_->Execute();
	//Present
	windowMaker_->PresentAllWindows();

	fenceManager_->SendSignal();

	ExecuteMessage();
	//バツを押されたら終了する
	exit_ = pushedCrossButton_;
}

void SHEngine::WaitForGPU() {
	fenceManager_->WaitForGPU();
}

void SHEngine::ImGuiActivate(Window* window) {
	imGuiForEngine_->Initialize(dxDevice_.get(), cmdListManager_.get(), window);
	imGuiForEngine_->BeginFrame();
	imGuiActive_ = true;
	imguiDrawed_ = false;

	logger_->info("ImGui activated");
}

void SHEngine::DrawImGui() {
	//ImGui
	if (imGuiActive_ && !imguiDrawed_) {
		imGuiForEngine_->EndFrame();
		imguiDrawed_ = true;
	}
}

void SHEngine::ExecuteMessage() {
	pushedCrossButton_ = false;

	while (msg_.message != WM_QUIT) {
		//メッセージがあれば処理する
		if (PeekMessage(&msg_, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg_);
			DispatchMessage(&msg_);
		} else {
			//メッセージがなければ処理を始める
			return;
		}
	}

	pushedCrossButton_ = true;
}

void SHEngine::FPSDraw() {
	if (!imGuiActive_) {
		return;
	}

#ifdef USE_IMGUI
	ImGui::Begin("FPS");
	ImGui::Text("CPU");
	ImGui::Text("FPS: %.2f ", 1.0f / fpsObserver_->GetDeltatime());
	ImGui::Text("deltaTime: %.2f ms", fpsObserver_->GetDeltatime() * 100.0f);
	ImGui::Separator();
	ImGui::Text("GPU");
	ImGui::Text("FPS: %.2f ", 1.0f / fpsObserver_->GetDeltatime(FPSType::GPU));
	ImGui::Text("deltaTime: %.2f ms", fpsObserver_->GetDeltatime(FPSType::GPU) * 100.0f);
	ImGui::End();
#endif
}
