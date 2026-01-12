#include "SneakWalk.h"
#include <imgui/imgui.h>

void SneakWalk::Initialize(ModelManager* modelManager, DrawDataManager* drawDataManager, Camera* camera) {
	renderObject_ = std::make_unique<RenderObject>("SneakWalk");
	renderObject_->Initialize();
	renderObject_->psoConfig_.inputLayoutID = InputLayoutID::Skinning;
	renderObject_->psoConfig_.vs = "Subj/SubjSkinning.VS.hlsl";
	renderObject_->psoConfig_.ps = "Subj/MultiPointLight.PS.hlsl";

	int LoadModelID = modelManager->LoadModel("SneakWalk");
	model_ = modelManager->GetSkinningModelData(LoadModelID);
	renderObject_->SetDrawData(drawDataManager->GetDrawData(model_.drawDataIndex));
	renderObject_->CreateCBV(sizeof(VSData), ShaderType::VERTEX_SHADER, "SneakWalk::VSData");
	int jointCount = static_cast<int>(model_.skeleton.joints.size());
	skinningMatrices_.resize(jointCount);
	renderObject_->CreateSRV(sizeof(WellForGPU), jointCount, ShaderType::VERTEX_SHADER, "SneakWalk::SkinningMatrices");
	renderObject_->CreateSRV(sizeof(PointLight), 4, ShaderType::PIXEL_SHADER, "SneakWalk::PointLights");

	animation_ = modelManager->LoadAnimation(0, "SneakWalk");

	pointLights_.resize(4);
	camera_ = camera;

	Load();
}

void SneakWalk::Update(float deltaTime) {
	//定数バッファ更新
	vsData_.worldMatrix = Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.position);
	vsData_.vpMatrix = camera_->GetVPMatrix();

	time += deltaTime;
	time = std::fmod(time, animation_.duration);

	AnimationUpdate(animation_, time, model_.skeleton);
	SkeletonUpdate(model_.skeleton);
	SkinningUpdate(skinningMatrices_, model_.skinClusterData, model_.skeleton);
}

void SneakWalk::Draw(Window* window) {
	renderObject_->CopyBufferData(0, &vsData_, sizeof(VSData));
	renderObject_->CopyBufferData(1, skinningMatrices_.data(), sizeof(WellForGPU) * skinningMatrices_.size());
	renderObject_->CopyBufferData(2, pointLights_.data(), sizeof(PointLight) * pointLights_.size());

	renderObject_->Draw(window);
}

void SneakWalk::DrawImGui() {
#ifdef USE_IMGUI

	ImGui::Begin("SneakWalk");

	ImGui::Text("Time : %.2f", time);
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.1f, 0.1f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("Position", &transform_.position.x, 0.1f);

	ImGui::Separator();

	ImGui::Text("Edit Index : %d", editIndex_);
	ImGui::SameLine();
	if (ImGui::Button("-")) {
		editIndex_ = std::max(0, editIndex_ - 1);
	}
	ImGui::SameLine();
	if (ImGui::Button("+")) {
		editIndex_ = std::min(static_cast<int>(pointLights_.size() - 1), editIndex_ + 1);
	}

	PointLight& light = pointLights_[editIndex_];

	ImGui::ColorEdit4("Light Color", &light.color.x);
	ImGui::DragFloat3("Light Position", &light.posiiton.x, 0.1f);
	ImGui::DragFloat("Light Intensity", &light.intensity, 0.1f, 0.0f);
	ImGui::DragFloat("Light Radius", &light.radius, 0.1f, 0.0f);
	ImGui::DragFloat("Light Decay", &light.decay, 0.1f, 0.0f);

	ImGui::End();

#endif
}

void SneakWalk::Save() {
	binaryManager_.RegistOutput(transform_.position);
	binaryManager_.RegistOutput(transform_.rotate);
	binaryManager_.RegistOutput(transform_.scale);

	for (int i = 0; i < pointLights_.size(); ++i) {
		binaryManager_.RegistOutput(pointLights_[i].color);
		binaryManager_.RegistOutput(pointLights_[i].posiiton);
		binaryManager_.RegistOutput(pointLights_[i].intensity);
		binaryManager_.RegistOutput(pointLights_[i].radius);
		binaryManager_.RegistOutput(pointLights_[i].decay);
	}

	binaryManager_.Write("SneakWalk.sg");
}

void SneakWalk::Load() {
	auto values = binaryManager_.Read("SneakWalk.sg");
	size_t index = 0;

	if (values.empty()) {
		return;
	}

	transform_.position = BinaryManager::Reverse<Vector3>(values[index++]);
	transform_.rotate = BinaryManager::Reverse<Vector3>(values[index++]);
	transform_.scale = BinaryManager::Reverse<Vector3>(values[index++]);

	pointLights_.resize(4);
	for (int i = 0; i < 4; ++i) {
		pointLights_[i].color = BinaryManager::Reverse<Vector4>(values[index++]);
		pointLights_[i].posiiton = BinaryManager::Reverse<Vector3>(values[index++]);
		pointLights_[i].intensity = BinaryManager::Reverse<float>(values[index++]);
		pointLights_[i].radius = BinaryManager::Reverse<float>(values[index++]);
		pointLights_[i].decay = BinaryManager::Reverse<float>(values[index++]);
	}
}
