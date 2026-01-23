#pragma once
#include <Render/RenderObject.h>
#include <Assets/Model/ModelManager.h>

class SneakWalk {
public:

	~SneakWalk() {
		Save();
	}

	void Initialize(ModelManager* modelManager, DrawDataManager* drawDataManager, Camera* camera);
	void Update(float deltaTime);
	void Draw(Window* window);
	void DrawImGui();

private:

	void Save();
	void Load();

	struct VSData {
		Matrix4x4 worldMatrix = Matrix4x4::Identity();
		Matrix4x4 vpMatrix = Matrix4x4::Identity();
	};
	std::vector<WellForGPU> skinningMatrices_;
	std::unique_ptr<RenderObject> renderObject_;
	Animation animation_{};
	VSData vsData_{};

	Transform transform_{};
	float time = 0.0f;
	float slideTime_ = 0.0f;

	struct PointLight {
		Vector4 color;
		Vector3 posiiton;
		float intensity;
		float radius;
		float decay;
		Vector2 pad;
	};
	std::vector<PointLight> pointLights_{};

	Camera* camera_ = nullptr;
	SkinningModelData model_{};

	int editIndex_ = 0;

	BinaryManager binaryManager_;
};
