#include "Sphere.h"
#include <imgui/imgui.h>

Sphere::~Sphere() {
	Save();
}

void Sphere::Initialize(DrawData& drawData, Camera* camera, int textureIndex) {
	obj_ = std::make_unique<RenderObject>("Sphere");
	obj_->Initialize();
	obj_->SetDrawData(drawData);
	obj_->CreateCBV(sizeof(VSData), ShaderType::VERTEX_SHADER, "Sphere_VS_CBV");
	obj_->CreateCBV(sizeof(PSData), ShaderType::PIXEL_SHADER, "Sphere_PS_CBV");

	obj_->psoConfig_.ps = "Subj/Subj.PS.hlsl";
	obj_->psoConfig_.vs = "Subj/Subj.VS.hlsl";
	obj_->SetUseTexture(true);

	camera_ = camera;
	psData_.textureIndex = textureIndex;

	transform_.position.y = 3.0f;
}

void Sphere::Draw(Window* window) {
	vsData_.world = Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.position);
	vsData_.vp = camera_->GetVPMatrix();
	vsData_.worldInv = vsData_.world.Inverse().Transpose();

	psData_.cameraPos = camera_->position_;

	// 定数バッファの更新
	obj_->CopyBufferData(0, &vsData_, sizeof(VSData));
	obj_->CopyBufferData(1, &psData_, sizeof(PSData));
	// 描画コマンドの発行
	obj_->Draw(window);
}

void Sphere::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("TestSphere");
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.1f, 0.1f, 10.0f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.1f, -360.0f, 360.0f);
	ImGui::DragFloat3("Position", &transform_.position.x, 0.1f, -100.0f, 100.0f);

	ImGui::Separator();

	ImGui::ColorEdit4("Base Color", &psData_.baseColor.x);
	ImGui::DragFloat("Shininess", &psData_.shininess, 1.0f, 256.0f);
	ImGui::DragFloat3("Light Direction", &psData_.dirLight.direction.x, 0.01f, -1.0f, 1.0f);
	psData_.dirLight.direction = psData_.dirLight.direction.Normalize();
	ImGui::ColorEdit4("Light Color", &psData_.dirLight.color.x);
	ImGui::DragFloat("Light Intensity", &psData_.dirLight.intensity, 0.0f, 10.0f);
	if (psData_.useBlinnPhong) {
		if (ImGui::Button("Use Phong")) {
			psData_.useBlinnPhong = 0;
		}
	} else {
		if (ImGui::Button("Use Blinn-Phong")) {
			psData_.useBlinnPhong = 1;
		}
	}
	ImGui::End();
#endif
}

void Sphere::Save() {
	binaryManager_.RegistOutput(transform_.position);
	binaryManager_.RegistOutput(transform_.rotate);
	binaryManager_.RegistOutput(transform_.scale);

	binaryManager_.RegistOutput(psData_.baseColor);
	binaryManager_.RegistOutput(psData_.dirLight.color);
	binaryManager_.RegistOutput(psData_.dirLight.direction);
	binaryManager_.RegistOutput(psData_.dirLight.intensity);

	binaryManager_.Write(saveFilePath_);
}

void Sphere::Load() {
	auto values = binaryManager_.Read(saveFilePath_);
	if (values.empty()) {
		return;
	}
	transform_.position = BinaryManager::Reverse<Vector3>(values[0]);
	transform_.rotate = BinaryManager::Reverse<Vector3>(values[1]);
	transform_.scale = BinaryManager::Reverse<Vector3>(values[2]);

	psData_.baseColor = BinaryManager::Reverse<Vector4>(values[3]);
	psData_.dirLight.color = BinaryManager::Reverse<Vector4>(values[4]);
	psData_.dirLight.direction = BinaryManager::Reverse<Vector3>(values[5]);
	psData_.dirLight.intensity = BinaryManager::Reverse<float>(values[6]);
}
