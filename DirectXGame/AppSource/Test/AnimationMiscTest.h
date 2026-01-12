#pragma once
#include <Render/RenderObject.h>
#include <Assets/Model/ModelManager.h>

class AnimationMiscTest {
public:

	~AnimationMiscTest() {
		Save();
	}

	void Initialize(ModelManager* modelManager, DrawDataManager* drawDataManager, Camera* camera);
	void Update(float deltaTime);
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

	Transform transform_;
	VSData vsData_;
	std::vector<Matrix4x4> matrices_;
	Camera* camera_ = nullptr;

	std::vector<std::vector<Animation>> animations_;
	Animation* currentAnimation_ = nullptr;
	std::vector<NodeModelData> nodeModels_;
	float timer_ = 0.0f;

	int currentAnimationIndex_ = 0;
	int currentSubAnimationIndex_ = 0;

	DrawDataManager* drawDataManager_ = nullptr;
	BinaryManager binaryManager_;
};
