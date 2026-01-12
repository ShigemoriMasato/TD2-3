#include "SpotLightTest.h"
#include <imgui/imgui.h>

void SpotLightTest::Initialize(DrawData& drawData, Camera* camera) {
	renderObject_ = std::make_unique<RenderObject>("SpotLightTest");
	renderObject_->Initialize();
	renderObject_->SetDrawData(drawData);
	renderObject_->psoConfig_.ps = "Subj/MultiSpotLight.PS.hlsl";
	renderObject_->psoConfig_.vs = "Subj/Subj.VS.hlsl";
	renderObject_->CreateCBV(sizeof(VSData), ShaderType::VERTEX_SHADER, "SpotLightTest VSData CBV");
	renderObject_->CreateSRV(sizeof(SpotLight), 4, ShaderType::PIXEL_SHADER, "SpotLightTest PSData SRV");

	spotLights_.resize(4);
	camera_ = camera;

	Load();

	vsData_.world = Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.position);
}

void SpotLightTest::Draw(Window* window) {
	vsData_.vp = camera_->GetVPMatrix();
	renderObject_->CopyBufferData(0, &vsData_, sizeof(VSData));
	renderObject_->CopyBufferData(1, spotLights_.data(), sizeof(SpotLight) * static_cast<size_t>(spotLights_.size()));
	renderObject_->Draw(window);
}

void SpotLightTest::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("SpotLightTest");

	ImGui::PushID(0);

	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.1f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("Position", &transform_.position.x, 0.1f);

	ImGui::PopID();

	vsData_.world = Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.position);

	ImGui::Separator();

	ImGui::PushID(1);

	ImGui::Text("Edit SpotLight Index : %d", editIndex_);
	ImGui::SameLine();
	if (ImGui::Button("-")) {
		editIndex_ = std::max(0, editIndex_ - 1);
	}
	ImGui::SameLine();
	if (ImGui::Button("+")) {
		editIndex_ = std::min(static_cast<int>(spotLights_.size()) - 1, editIndex_ + 1);
	}

	SpotLight& light = spotLights_[editIndex_];
	ImGui::ColorEdit4("Color", &light.color.x);
	ImGui::DragFloat3("Position", &light.position.x, 0.1f);
	ImGui::DragFloat3("Direction", &light.direction.x, 0.1f, -1.0f, 1.0f);
	light.direction = light.direction.Normalize();
	ImGui::DragFloat("Intensity", &light.intensity, 0.1f);
	ImGui::DragFloat("Distance", &light.distance, 0.1f);
	ImGui::DragFloat("Decay", &light.decay, 0.1f);
	ImGui::DragFloat("CosAngle", &light.cosAngle, 0.01f);
	ImGui::DragFloat("Attenuation", &light.attenuation, 0.1f);
	ImGui::DragFloat("FalloffStart", &light.falloffStart, 0.1f);

	ImGui::PopID();

	ImGui::End();
#endif
}

void SpotLightTest::Save() {
	binaryManager_.RegistOutput(transform_.scale);
	binaryManager_.RegistOutput(transform_.rotate);
	binaryManager_.RegistOutput(transform_.position);

	for (int i = 0; i < 4; ++i) {
		binaryManager_.RegistOutput(spotLights_[i].color);
		binaryManager_.RegistOutput(spotLights_[i].position);
		binaryManager_.RegistOutput(spotLights_[i].intensity);
		binaryManager_.RegistOutput(spotLights_[i].direction);
		binaryManager_.RegistOutput(spotLights_[i].distance);
		binaryManager_.RegistOutput(spotLights_[i].decay);
		binaryManager_.RegistOutput(spotLights_[i].cosAngle);
		binaryManager_.RegistOutput(spotLights_[i].attenuation);
		binaryManager_.RegistOutput(spotLights_[i].falloffStart);
	}

	binaryManager_.Write("SpotLightTest.sg");
}

void SpotLightTest::Load() {
	auto values = binaryManager_.Read("SpotLightTest.sg");
	if (values.empty()) {
		return;
	}

	size_t index = 0;
	transform_.scale = BinaryManager::Reverse<Vector3>(values[index++]);
	transform_.rotate = BinaryManager::Reverse<Vector3>(values[index++]);
	transform_.position = BinaryManager::Reverse<Vector3>(values[index++]);
	for (int i = 0; i < 4; ++i) {
		spotLights_[i].color = BinaryManager::Reverse<Vector4>(values[index++]);
		spotLights_[i].position = BinaryManager::Reverse<Vector3>(values[index++]);
		spotLights_[i].intensity = BinaryManager::Reverse<float>(values[index++]);
		spotLights_[i].direction = BinaryManager::Reverse<Vector3>(values[index++]);
		spotLights_[i].distance = BinaryManager::Reverse<float>(values[index++]);
		spotLights_[i].decay = BinaryManager::Reverse<float>(values[index++]);
		spotLights_[i].cosAngle = BinaryManager::Reverse<float>(values[index++]);
		spotLights_[i].attenuation = BinaryManager::Reverse<float>(values[index++]);
		spotLights_[i].falloffStart = BinaryManager::Reverse<float>(values[index++]);
	}
}
