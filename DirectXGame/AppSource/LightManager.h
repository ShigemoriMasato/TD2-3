#pragma once
#include"Utility/Vector.h"
#include <imgui/imgui.h>

// 平行光源の構造体
struct DirectionalLight {
	Vector4 color = { 1.f, 1.f, 1.f, 1.f };
	Vector3 direction = { 0.f,-1.f,0.f };
	float intensity = 0.0f;
};

struct PointLight {
	Vector4 color = { 1.f, 1.f, 1.f, 1.f };
	Vector3 position = {};
	float intensity = 1.0f;
	float radius = 3.0f;
	float decay = 1.0f;
	Vector2 pad;
};

class LightManager {
public:

	static LightManager* GetInstance() {
		static LightManager instance = LightManager();
		return &instance;
	}

	void Finalize() {
		for (const auto& dirLight : dirLights_) {
			binaryManager_.RegisterOutput(dirLight.color);
			binaryManager_.RegisterOutput(dirLight.direction);
			binaryManager_.RegisterOutput(dirLight.intensity);
		}
		//PointLightは毎度作成する
		binaryManager_.Write("LightConfig");
	}

	void Load() {
		auto values = binaryManager_.Read("LightConfig");
		dirLights_.clear();
		int index = 0;
		while (index < values.size()) {
			DirectionalLight dirLight;
			dirLight.color = binaryManager_.Reverse<Vector4>(values[index++].get());
			dirLight.direction = binaryManager_.Reverse<Vector3>(values[index++].get());
			dirLight.intensity = binaryManager_.Reverse<float>(values[index++].get());
			dirLights_.push_back(dirLight);
		}
	}

	int AddPointLight(Vector3* position, PointLight pointLight) {
		for (size_t i = 0; i < pointLightActive_.size(); ++i) {
			if (!pointLightActive_[i]) {
				pointLights_[i] = pointLight;
				pointPositions_[i] = position;
				pointLightActive_[i] = true;
				return static_cast<int>(i);
			}
		}

		pointPositions_.push_back(position);
		pointLights_.push_back(pointLight);
		pointLightActive_.push_back(true);
		return static_cast<int>(pointLights_.size() - 1);
	}

	void SetCameraPos(const Vector3& cameraPos) {
		cameraPos_ = cameraPos;
	}

	//PointLightの位置更新。最後推奨
	void Update() {
		for (size_t i = 0; i < pointLights_.size(); ++i) {
			if (pointLightActive_[i])
				pointLights_[i].position = *pointPositions_[i];
		}
	}

	void RemovePointLight(int index) {
		if (index < 0 || index >= static_cast<int>(pointLightActive_.size())) {
			return;
		}
		pointLightActive_[index] = false;
	}

	void DrawImGui() {
#ifdef USE_IMGUI

		ImGui::Begin("Light Manager");
		ImGui::BeginChild("Directional Light Number Select", { 100, 0 });

		for (int i = 0; i < 8; ++i) {
			if (ImGui::Button(std::to_string(i).c_str())) {
				editingDirLightIndex_ = i;
			}
			if (i % 3 != 2) {
				ImGui::SameLine();
			}
		}

		ImGui::EndChild();

		ImGui::SameLine();

		DirectionalLight& editLight = dirLights_[editingDirLightIndex_];
		ImGui::BeginChild("Directional Light Edit");
		ImGui::Text("Editing Directional Light %d", editingDirLightIndex_);
		ImGui::DragFloat3("Direction", &editLight.direction.x, 0.1f, -1.0f, 1.0f);
		ImGui::DragFloat("Intensity", &editLight.intensity, 0.1f, 0.0f, 10.0f);
		ImGui::ColorEdit4("Color", &editLight.color.x);
		ImGui::EndChild();
		ImGui::End();

		editLight.direction = editLight.direction.Normalize();


		ImGui::Begin("PointLightTest");

		static Vector3 testPos = { 0.0f, 0.0f, 0.0f };
		static int pointLightIndex = AddPointLight(&testPos, PointLight{});

		ImGui::DragFloat3("PointLight Position", &testPos.x, 0.1f);
		ImGui::DragFloat("PointLight Intensity", &pointLights_[pointLightIndex].intensity, 0.1f, 0.0f, 10.0f);
		ImGui::DragFloat("PointLight Radius", &pointLights_[pointLightIndex].radius, 0.1f, 0.1f, 100.0f);
		ImGui::DragFloat("PointLight Decay", &pointLights_[pointLightIndex].decay, 0.1f, 0.0f, 10.0f);
		ImGui::ColorEdit4("PointLight Color", &pointLights_[pointLightIndex].color.x);

		ImGui::End();
#endif
	}


	const std::vector<DirectionalLight>& GetDirLights() const {
		return dirLights_;
	}
	const std::vector<PointLight>& GetPointLights() const {
		return pointLights_;
	}
	PointLight& GetPointLight(int index) { return pointLights_[index]; }
	Vector3 GetCameraPos() const {
		return cameraPos_;
	}

private:

	std::vector<Vector3*> pointPositions_;

	std::vector<DirectionalLight> dirLights_ = std::vector<DirectionalLight>(8, {});
	std::vector<bool> pointLightActive_;
	std::vector<PointLight> pointLights_;

	Vector3 cameraPos_;

	BinaryManager binaryManager_;

	int editingDirLightIndex_ = 0;
};