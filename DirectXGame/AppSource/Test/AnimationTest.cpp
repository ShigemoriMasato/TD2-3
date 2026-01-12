#include "AnimationTest.h"
#include <imgui/imgui.h>
#include <format>

void AnimationTest::Initialize(ModelManager* modelManager, DrawDataManager* drawDataManager, Camera* camera) {
	std::string directory = "Assets/Model/Animation_Node";
	std::string baseName = "Animation_Node_";
	std::string extension = ".gltf";

	for (int i = 0; i < 6; ++i) {
		std::string num = std::format("{:02}", i);

		int index = modelManager->LoadModel(directory, baseName + num + extension);
		NodeModelData model = modelManager->GetNodeModelData(index);
		nodeModels_.push_back(model);

		Animation animation = modelManager->LoadAnimation(0, directory, baseName + num + extension);
		animations_.push_back(animation);
	}

	camera_ = camera;

	renderObject_ = std::make_unique<RenderObject>("AnimationTest");
	renderObject_->Initialize();
	renderObject_->psoConfig_.ps = "PostEffect/Simple.PS.hlsl";
	renderObject_->psoConfig_.vs = "Simple.VS.hlsl";
	renderObject_->SetUseTexture(true);
	renderObject_->CreateCBV(sizeof(VSData), ShaderType::VERTEX_SHADER, "AnimationTest VSData CBV");
	renderObject_->CreateCBV(sizeof(int), ShaderType::PIXEL_SHADER, "AnimationTest PSData CBV");
	
	Load();

	renderObject_->CopyBufferData(1, &nodeModels_[currentAnimationIndex_].materials.front().textureIndex, sizeof(int));
	currentAnimation_ = &animations_[currentAnimationIndex_];
	renderObject_->SetDrawData(drawDataManager->GetDrawData(nodeModels_[currentAnimationIndex_].drawDataIndices));
	drawDataManager_ = drawDataManager;
}

void AnimationTest::Update(float deltaTime) {
	if (currentAnimation_ == nullptr) {
		return;
	}
	timer_ += deltaTime;
	timer_ = std::fmod(timer_, currentAnimation_->duration);
	Matrix4x4 animationMat = AnimationUpdate(*currentAnimation_, timer_, nodeModels_[currentAnimationIndex_].rootNode);
	vsData_.world = animationMat * Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.position);
}

void AnimationTest::Draw(Window* window) {
	if (currentAnimation_ == nullptr) {
		return;
	}
	vsData_.vp = camera_->GetVPMatrix();
	renderObject_->CopyBufferData(0, &vsData_, sizeof(VSData));
	renderObject_->Draw(window);
}

void AnimationTest::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("NodeAnimationTest");

	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.1f, 0.1f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("Position", &transform_.position.x, 0.1f);

	ImGui::Separator();

	ImGui::Text("Timer : %.2f", timer_);
	ImGui::Text("Animation : %d", currentAnimationIndex_);
	ImGui::SameLine();
	if (ImGui::Button("-")) {
		currentAnimationIndex_ = std::max(0, currentAnimationIndex_ - 1);
	}
	ImGui::SameLine();
	if (ImGui::Button("+")) {
		currentAnimationIndex_ = std::min(static_cast<int>(animations_.size()) - 1, currentAnimationIndex_ + 1);
	}

	currentAnimation_ = &animations_[currentAnimationIndex_];
	renderObject_->SetDrawData(drawDataManager_->GetDrawData(nodeModels_[currentAnimationIndex_].drawDataIndices));

	ImGui::End();
#endif
}

void AnimationTest::Save() {
	binaryManager_.RegistOutput(transform_.scale);
	binaryManager_.RegistOutput(transform_.rotate);
	binaryManager_.RegistOutput(transform_.position);
	binaryManager_.RegistOutput(currentAnimationIndex_);
	binaryManager_.Write("NodeAnimationTest.sg");
}

void AnimationTest::Load() {
	auto values = binaryManager_.Read("NodeAnimationTest.sg");
	if (values.empty()) {
		return;
	}
	int index = 0;
	transform_.scale = BinaryManager::Reverse<Vector3>(values[index++]);
	transform_.rotate = BinaryManager::Reverse<Vector3>(values[index++]);
	transform_.position = BinaryManager::Reverse<Vector3>(values[index++]);
	currentAnimationIndex_ = BinaryManager::Reverse<int>(values[index++]);
}
