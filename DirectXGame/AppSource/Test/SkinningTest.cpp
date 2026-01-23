#include "SkinningTest.h"
#include <format>
#include <imgui/imgui.h>

void SkinningTest::Initialize(ModelManager* modelManager, DrawDataManager* drawDataManager, Camera* camera) {
	camera_ = camera;
	drawDataManager_ = drawDataManager;
	
	for (int i = 0; i < 12; ++i) {
		std::string number{};
		if (i != 3) {
			number = std::format("{:02}", i);
		} else {
			number = "00";
		}
		std::string name = "Animation_Skin_" + number + ".gltf";

		int modelID = modelManager->LoadModel("Animation_Skin", name);
		models_.push_back(modelManager->GetSkinningModelData(modelID));
		animations_.push_back(modelManager->LoadAnimation(0, "Animation_Skin", name));
	}

	renderObject_ = std::make_unique<RenderObject>("SkinningTest");
	renderObject_->Initialize();
	renderObject_->psoConfig_.inputLayoutID = InputLayoutID::Skinning;
	renderObject_->psoConfig_.vs = "Subj/SubjSkinning.VS.hlsl";
	renderObject_->psoConfig_.ps = "PostEffect/Simple.PS.hlsl";

	int LoadModelID = modelManager->LoadModel("SneakWalk");
	renderObject_->CreateCBV(sizeof(VSData), ShaderType::VERTEX_SHADER, "SneakWalk::VSData");
	renderObject_->CreateSRV(sizeof(WellForGPU), 128, ShaderType::VERTEX_SHADER, "SneakWalk::SkinningMatrices");
	renderObject_->SetUseTexture(true);
	renderObject_->CreateCBV(sizeof(int), ShaderType::PIXEL_SHADER, "SkinningTest::TextureIndex");
	skinningMatrices_.reserve(128);

	Load();

	auto drawData = drawDataManager_->GetDrawData(models_[currentAnimationIndex_].drawDataIndex);
	renderObject_->SetDrawData(drawData);
	int jointCount = static_cast<int>(models_[currentAnimationIndex_].skeleton.joints.size());
	skinningMatrices_.resize(jointCount);
}

void SkinningTest::Update(float deltaTime) {
	vsData_.worldMatrix = Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.position);
	vsData_.vpMatrix = camera_->GetVPMatrix();

	SkeletonUpdate(models_[currentAnimationIndex_].skeleton);
	SkinningUpdate(skinningMatrices_, models_[currentAnimationIndex_].skinClusterData, models_[currentAnimationIndex_].skeleton);
	timer_ += deltaTime;
	if (animations_[currentAnimationIndex_].duration > 0.0f) {
		timer_ = fmodf(timer_, animations_[currentAnimationIndex_].duration);
	} else {
		timer_ = 0.0f;
	}
	AnimationUpdate(animations_[currentAnimationIndex_], timer_, models_[currentAnimationIndex_].skeleton);
}

void SkinningTest::Draw(Window* window) {
	if (animations_.empty()) {
		return;
	}
	renderObject_->CopyBufferData(0, &vsData_, sizeof(VSData));
	renderObject_->CopyBufferData(1, skinningMatrices_.data(), sizeof(WellForGPU) * skinningMatrices_.size());
	int textureIndex = models_[currentAnimationIndex_].materials.front().textureIndex;
	renderObject_->CopyBufferData(2, &textureIndex, sizeof(int));
	renderObject_->Draw(window);
}

void SkinningTest::DrawImGui() {
#ifdef USE_IMGUI

	ImGui::Begin("SkinningTest");

	ImGui::Text("Time : %.2f", timer_);
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.1f, 0.1f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("Position", &transform_.position.x, 0.1f);
	ImGui::Separator();
	ImGui::Text("Animation : %d", currentAnimationIndex_);
	ImGui::SameLine();
	if (ImGui::Button("-")) {
		currentAnimationIndex_ = std::max(0, currentAnimationIndex_ - 1);
		auto drawData = drawDataManager_->GetDrawData(models_[currentAnimationIndex_].drawDataIndex);
		renderObject_->SetDrawData(drawData);
		int jointCount = static_cast<int>(models_[currentAnimationIndex_].skeleton.joints.size());
		skinningMatrices_.resize(jointCount);
	}
	ImGui::SameLine();
	if (ImGui::Button("+")) {
		currentAnimationIndex_ = std::min(static_cast<int>(animations_.size()) - 1, currentAnimationIndex_ + 1);
		auto drawData = drawDataManager_->GetDrawData(models_[currentAnimationIndex_].drawDataIndex);
		renderObject_->SetDrawData(drawData);
		int jointCount = static_cast<int>(models_[currentAnimationIndex_].skeleton.joints.size());
		skinningMatrices_.resize(jointCount);
	}

	ImGui::End();

#endif
}

void SkinningTest::Load() {
	auto values = binaryManager_.Read(saveFile_);

	if (values.empty()) {
		return;
	}

	uint32_t index = 0;
	transform_.scale = BinaryManager::Reverse<Vector3>(values[index++]);
	transform_.rotate = BinaryManager::Reverse<Vector3>(values[index++]);
	transform_.position = BinaryManager::Reverse<Vector3>(values[index++]);
	currentAnimationIndex_ = BinaryManager::Reverse<int>(values[index++]);
}

void SkinningTest::Save() {
	binaryManager_.RegistOutput(transform_.scale);
	binaryManager_.RegistOutput(transform_.rotate);
	binaryManager_.RegistOutput(transform_.position);
	binaryManager_.RegistOutput(currentAnimationIndex_);
	binaryManager_.Write(saveFile_);
}
