#pragma once
#include <Render/RenderObject.h>
#include <Utility/DataStructures.h>

class Sphere {
public:

	~Sphere();

	void Initialize(DrawData& drawData, Camera* camera, int textureIndex);
	void Draw(Window* window);
	void DrawImGui();

private:

	void Save();
	void Load();

	std::unique_ptr<RenderObject> obj_ = nullptr;

	struct DirectionalLight {
		Vector4 color = { 1.0f,1.0f,1.0f,1.0f };
		Vector3 direction = { 0.0f,-1.0f,0.0f };
		float intensity = 1.0f;
	};

	struct PSData {
		Vector4 baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		DirectionalLight dirLight;
		Vector3 cameraPos;
		float shininess = 32.0f;
		int textureIndex;
		int useBlinnPhong;
		int pad[2];
	};

	struct VSData {
		Matrix4x4 world;
		Matrix4x4 vp;
	};

	PSData psData_{};
	VSData vsData_{};

	Transform transform_{};
	Camera* camera_ = nullptr;

	BinaryManager binaryManager_;
	std::string saveFilePath_ = "Sphere.sg";

};
