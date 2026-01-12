#pragma once
#include <Render/RenderObject.h>
#include <Utility/DataStructures.h>
#include <Assets/Model/ModelManager.h>

class SamplerTest {
public:

	~SamplerTest();

	void Initialize(ModelManager* modelManager, DrawDataManager* drawDataManager, Camera* camera);
	void Draw(Window* window);
	void DrawImGui();

private:

	void Save();
	void Load();

	Camera* camera_ = nullptr;
	std::unique_ptr<RenderObject> renderObject_;

	struct VSData {
		Matrix4x4 world;
		Matrix4x4 vp;
	};
	VSData vsData_;

	std::map<SamplerID, std::string> samplerNames_;
	std::vector<std::string> modelNames_;
	std::string directorypath_ = "Assets/Model/Texture_Sampler";
	int currentSamplerIndex_ = 0;

	Transform transform_;

	BinaryManager binaryManager_;
	std::string saveFile_ = "SamplerTest.sg";

	ModelManager* modelManager_ = nullptr;
	DrawDataManager* drawDataManager_ = nullptr;
};
