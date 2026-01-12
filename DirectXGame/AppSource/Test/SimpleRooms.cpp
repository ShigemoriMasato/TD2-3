#include "SimpleRooms.h"
#include <Utility/Easing.h>
#include <imgui/imgui.h>

void SimpleRooms::Initialize(DrawData& drawData, Camera* camera) {
	obj_ = std::make_unique<RenderObject>("SimpleRooms");

	const int roomNum = 8;

	obj_->Initialize();
	obj_->SetDrawData(drawData);
	obj_->CreateSRV(sizeof(VSData), roomNum, ShaderType::VERTEX_SHADER, "SimpleRooms_VS_SRV");
	obj_->CreateSRV(sizeof(Vector4), roomNum, ShaderType::PIXEL_SHADER, "SimpleRooms_PS_SRV");
	obj_->CreateCBV(sizeof(DirectionalLight), ShaderType::PIXEL_SHADER, "SimpleRooms_PS_CBV");
	obj_->psoConfig_.ps = "Subj/SimpleRoom.PS.hlsl";
	obj_->psoConfig_.vs = "Subj/SimpleRoom.VS.hlsl";

	std::vector<Vector4> colorMap;
	Vector4 beginColor = { 1.0f, 0.0f, 0.0f, 1.0f };
	Vector4 endColor = { 0.0f, 1.0f, 1.0f, 1.0f };
	for(int i = 0; i < roomNum; ++i) {
		VSData vsData{};
		vsData.world = Matrix::MakeTranslationMatrix(Vector3{ (7.f * float(i)), 0.0f, 0.0f });
		roomMatrices_.push_back(vsData);
		float t = float(i) / float(roomNum - 1);
		Vector4 color = lerp(beginColor, endColor, t);
		colorMap.push_back(color);
	}

	obj_->CopyBufferData(1, colorMap.data(), sizeof(Vector4) * colorMap.size());
	obj_->instanceNum_ = roomNum;
	camera_ = camera;
}

void SimpleRooms::Draw(Window* window) {
	for(auto& mat : roomMatrices_) {
		mat.vp = camera_->GetVPMatrix();
	}
	obj_->CopyBufferData(0, roomMatrices_.data(), sizeof(VSData) * roomMatrices_.size());
	obj_->CopyBufferData(2, &light_, sizeof(DirectionalLight));

	obj_->Draw(window);
}

void SimpleRooms::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("SimpleRooms");
	ImGui::ColorEdit4("Light Color", &light_.color.x);
	ImGui::DragFloat3("Light Direction", &light_.direction.x, 0.01f, -1.0f, 1.0f);
	light_.direction = light_.direction.Normalize();
	ImGui::DragFloat("Light Intensity", &light_.intensity, 0.1f);
	ImGui::End();
#endif
}
