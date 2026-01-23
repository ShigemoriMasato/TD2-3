#pragma once
#include <Render/RenderObject.h>
#include <Assets/Model/ModelManager.h>
#include <Camera/Camera.h>

class SkinningTest {
public:

	~SkinningTest() {
		Save();
	}

	void Initialize(ModelManager* modelManager, DrawDataManager* drawDataManager, Camera* camera);
	void Update(float deltaTime);
	void Draw(Window* window);
	void DrawImGui();

private:

	void Load();
	void Save();

	struct VSData {
		Matrix4x4 worldMatrix = Matrix4x4::Identity();
		Matrix4x4 vpMatrix = Matrix4x4::Identity();
	};
	std::vector<WellForGPU> skinningMatrices_;
	std::unique_ptr<RenderObject> renderObject_;
	std::vector<Animation> animations_{};
	VSData vsData_{};

	std::vector<SkinningModelData> models_{};

	float timer_ = 0.0f;
	int currentAnimationIndex_ = 0;

	Transform transform_{};

	Camera* camera_ = nullptr;
	DrawDataManager* drawDataManager_;

	BinaryManager binaryManager_;
	std::string saveFile_ = "SkinningTest.sg";

};
