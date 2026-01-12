#include "AnimationMiscTest.h"
#include <format>
#include <imgui/imgui.h>

void AnimationMiscTest::Initialize(ModelManager* modelManager, DrawDataManager* drawDataManager, Camera* camera) {
	camera_ = camera;
	drawDataManager_ = drawDataManager;

	std::string directory = "Assets/Model/Animation_NodeMisc";
	std::string baseName = "Animation_NodeMisc_";
	std::string extension = ".gltf";

	animations_.resize(9);
	nodeModels_.resize(9);
	for (int i = 0; i < 9; ++i) {
		std::string num = std::format("{:02}", i);
		std::string factName = baseName + num + extension;

		int modelID = modelManager->LoadModel(directory, factName);
		nodeModels_[i] = modelManager->GetNodeModelData(modelID);

		for (int j = 0; j < 2; ++j) {
			animations_[i].push_back(modelManager->LoadAnimation(0, directory, factName));
		}
	}

	// RenderObjectの作成
	renderObject_ = std::make_unique<RenderObject>();
	renderObject_->Initialize();
	renderObject_->psoConfig_.ps = "PostEffect/Simple.PS.hlsl";
	renderObject_->psoConfig_.vs = "Simple.VS.hlsl";
	renderObject_->SetUseTexture(true);
	renderObject_->CreateCBV(sizeof(VSData), ShaderType::VERTEX_SHADER, "AnimationTest VSData CBV");
	renderObject_->CreateCBV(sizeof(int), ShaderType::PIXEL_SHADER, "AnimationTest PSData CBV");

	Load();

	renderObject_->CopyBufferData(1, &nodeModels_[currentAnimationIndex_].materials.front().textureIndex, sizeof(int));
	currentAnimation_ = &animations_[currentAnimationIndex_][currentSubAnimationIndex_];
	renderObject_->SetDrawData(drawDataManager->GetDrawData(nodeModels_[currentAnimationIndex_].drawDataIndices));
	drawDataManager_ = drawDataManager;

	matrices_.resize(4);
}

void AnimationMiscTest::Update(float deltaTime) {
	if (currentAnimation_ == nullptr) {
		return;
	}
	timer_ += deltaTime;
	timer_ = std::fmod(timer_, currentAnimation_->duration);

	auto& model = nodeModels_[currentAnimationIndex_];

	vsData_.world = Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.position);
}

void AnimationMiscTest::Draw(Window* window) {
	if (!renderObject_) return;
	vsData_.vp = camera_->GetVPMatrix();
	renderObject_->CopyBufferData(0, &vsData_, sizeof(VSData));
	renderObject_->Draw(window);
}

void AnimationMiscTest::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("NodeAnimationTest");

	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.1f, 0.1f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("Position", &transform_.position.x, 0.1f);

	ImGui::Separator();

	ImGui::Text("Timer : %.2f", timer_);
	ImGui::Text("Animation : %d", currentAnimationIndex_);
	ImGui::SameLine();
	ImGui::PushID(0);
	if (ImGui::Button("-")) {
		currentAnimationIndex_ = std::max(0, currentAnimationIndex_ - 1);
	}
	ImGui::SameLine();
	if (ImGui::Button("+")) {
		currentAnimationIndex_ = std::min(static_cast<int>(animations_.size()) - 1, currentAnimationIndex_ + 1);
	}
	ImGui::PopID();
	ImGui::Text("Sub Animation : %d", currentSubAnimationIndex_);
	ImGui::PushID(1);
	ImGui::SameLine();
	if (ImGui::Button("-")) {
		currentSubAnimationIndex_ = std::max(0, currentSubAnimationIndex_ - 1);
	}
	ImGui::SameLine();
	if (ImGui::Button("+")) {
		currentSubAnimationIndex_ = std::min(static_cast<int>(animations_[currentAnimationIndex_].size()) - 1, currentSubAnimationIndex_ + 1);
	}
	ImGui::PopID();

	currentAnimation_ = &animations_[currentAnimationIndex_][currentSubAnimationIndex_];
	renderObject_->SetDrawData(drawDataManager_->GetDrawData(nodeModels_[currentAnimationIndex_].drawDataIndices));

	ImGui::End();
#endif
}

void AnimationMiscTest::Save() {
	binaryManager_.RegistOutput(transform_.scale);
	binaryManager_.RegistOutput(transform_.rotate);
	binaryManager_.RegistOutput(transform_.position);
	binaryManager_.RegistOutput(currentAnimationIndex_);
	binaryManager_.RegistOutput(currentSubAnimationIndex_);
	binaryManager_.Write("AnimationMiscTest.sg");
}

void AnimationMiscTest::Load() {
	auto values = binaryManager_.Read("AnimationMiscTest.sg");
	if (values.size() != 5) {
		return;
	}
	transform_.scale = BinaryManager::Reverse<Vector3>(values[0]);
	transform_.rotate = BinaryManager::Reverse<Vector3>(values[1]);
	transform_.position = BinaryManager::Reverse<Vector3>(values[2]);
	currentAnimationIndex_ = BinaryManager::Reverse<int>(values[3]);
	currentSubAnimationIndex_ = BinaryManager::Reverse<int>(values[4]);
}
