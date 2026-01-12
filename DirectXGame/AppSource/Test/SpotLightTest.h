#pragma once
#include <Render/RenderObject.h>
#include <Utility/DataStructures.h>

class SpotLightTest {
public:

	~SpotLightTest() {
		Save();
	}

	void Initialize(DrawData& drawData, Camera* camera);
	void Draw(Window* window);
	void DrawImGui();

private:

	void Save();
	void Load();

	std::unique_ptr<RenderObject> renderObject_;

	struct VSData {
		Matrix4x4 world;
		Matrix4x4 vp;
	};
	VSData vsData_;
	Transform transform_;
	Camera* camera_ = nullptr;

	struct SpotLight {
		Vector4 color;
		Vector3 position;
		float intensity;
		Vector3 direction;
		float distance;
		float decay;
		float cosAngle;
		float attenuation;
		float falloffStart;
	};
	std::vector<SpotLight> spotLights_;
	int editIndex_ = 0;

	BinaryManager binaryManager_;
};
