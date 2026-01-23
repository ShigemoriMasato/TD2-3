#include "TestScene.h"
#include <imgui/imgui.h>

namespace {
	bool debug = false;

	std::string simpleSkin = "Assets/.EngineResource/Model/SimpleSkin";
	std::string sneakWalk = "SneakWalk";
}

void TestScene::Initialize() {
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
	debugCamera_->SetCenter({ 0.0f, 3.0f, 0.0f });

	sphere_ = std::make_unique<Sphere>();
	int loadModelID = modelManager_->LoadModel("MonsterBall");
	auto nodeModel = modelManager_->GetNodeModelData(loadModelID);
	auto drawData = drawDataManager_->GetDrawData(nodeModel.drawDataIndices);
	int textureIndex = nodeModel.materials.back().textureIndex;
	sphere_->Initialize(drawData.front(), debugCamera_.get(), textureIndex);

	simpleRooms_ = std::make_unique<SimpleRooms>();
	nodeModel = modelManager_->GetNodeModelData(modelManager_->LoadModel("SimpleRoom"));
	drawData = drawDataManager_->GetDrawData(nodeModel.drawDataIndices);
	simpleRooms_->Initialize(drawData.front(), debugCamera_.get());

	sneakWalk_ = std::make_unique<SneakWalk>();
	sneakWalk_->Initialize(modelManager_, drawDataManager_, debugCamera_.get());

	samplerTest_ = std::make_unique<SamplerTest>();
	samplerTest_->Initialize(modelManager_, drawDataManager_, debugCamera_.get());

	animationTest_ = std::make_unique<AnimationTest>();
	animationTest_->Initialize(modelManager_, drawDataManager_, debugCamera_.get());

	animationMiscTest_ = std::make_unique<AnimationMiscTest>();
	animationMiscTest_->Initialize(modelManager_, drawDataManager_, debugCamera_.get());

	spotLightTest_ = std::make_unique<SpotLightTest>();
	drawData = drawDataManager_->GetDrawData(modelManager_->GetNodeModelData(1).drawDataIndices);
	spotLightTest_->Initialize(drawData.front(), debugCamera_.get());

	skinningTest_ = std::make_unique<SkinningTest>();
	skinningTest_->Initialize(modelManager_, drawDataManager_, debugCamera_.get());

	labels_ = {
		"Sphere + Reflection",
		"Skinning + Multi PointLight",
		"Sampler Test",
		"Animation Test",
		"Animation Misc Test(Unfinished)",
		"Multi SpotLight Test + Plane.obj",
		"Skinning Test(Unfinished)",
	};
}

std::unique_ptr<IScene> TestScene::Update() {
	input_->Update();
	debugCamera_->Update();

	float deltaTime = engine_->GetFPSObserver()->GetDeltatime() * timeRatio_;

	sneakWalk_->Update(deltaTime);

	animationTest_->Update(deltaTime);

	animationMiscTest_->Update(deltaTime);

	skinningTest_->Update(deltaTime);

	return nullptr;
}

void TestScene::Draw() {
	auto window = commonData_->mainWindow.get();
	auto display = commonData_->display.get();

	display->PreDraw(window->GetCommandList(), true);
	simpleRooms_->Draw(window->GetWindow());
	sneakWalk_->Draw(window->GetWindow());
	sphere_->Draw(window->GetWindow());
	animationTest_->Draw(window->GetWindow());
	animationMiscTest_->Draw(window->GetWindow());
	samplerTest_->Draw(window->GetWindow());
	spotLightTest_->Draw(window->GetWindow());
	skinningTest_->Draw(window->GetWindow());
	display->PostDraw(window->GetCommandList());

	window->PreDraw();
	window->PostDraw();
	//ImGui
#ifdef USE_IMGUI
	ImGui::Begin("TestScene");
	for (int i = 0; i < labels_.size(); ++i) {
		if (ImGui::RadioButton(labels_[i].c_str(), &editIndex_, i)) {
			editIndex_ = i;
			debugCamera_->Initialize();
			debugCamera_->SetCenter({ 7.0f * float(i), 3.0f, 0.0f});
			break;
		}
	}
	ImGui::End();

	simpleRooms_->DrawImGui();

	switch (editIndex_) {
	case 0:
		sphere_->DrawImGui();
		break;
	case 1:
		sneakWalk_->DrawImGui();
		break;
	case 2:
		samplerTest_->DrawImGui();
		break;
	case 3:
		animationTest_->DrawImGui();
		break;
	case 4:
		animationMiscTest_->DrawImGui();
		break;
	case 5:
		spotLightTest_->DrawImGui();
		break;
	case 6:
		skinningTest_->DrawImGui();
		break;
	}
#endif

	ImGui::Begin("Time");
	ImGui::DragFloat("Time Ratio", &timeRatio_, 0.01f, 0.0f);
	ImGui::End();
	ImGui::Begin("FPS");
	ImGui::DragInt("Max FPS", &maxFPS_, 1, 1, 240);
	ImGui::End();

	engine_->GetFPSObserver()->SetTargetFPS(double(maxFPS_));

	engine_->DrawImGui();
}
