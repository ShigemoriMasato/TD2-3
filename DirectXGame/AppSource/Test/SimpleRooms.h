#pragma once
#include <Render/RenderObject.h>

class SimpleRooms {
public:

	void Initialize(DrawData& drawData, Camera* camera);
	void Draw(Window* window);
	void DrawImGui();

private:

	struct DirectionalLight {
		Vector4 color = { 1.0f,1.0f,1.0f,1.0f };
		Vector3 direction = { 0.0f,-1.0f,0.0f };
		float intensity = 1.0f;
	};

	struct VSData {
		Matrix4x4 world;
		Matrix4x4 vp;
	};

	std::vector<VSData> roomMatrices_;
	std::unique_ptr<RenderObject> obj_ = nullptr;
	DirectionalLight light_{};
	Camera* camera_ = nullptr;
};
