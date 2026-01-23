#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <assimp/scene.h>
#include <Tool/Logger/Logger.h>
#include <Render/DrawDataManager.h>
#include <assimp/Importer.hpp>

#include "ModelData.h"

class TextureManager;

class ModelManager {
public:

	ModelManager() = default;
	~ModelManager() = default;

	void Initialize(TextureManager* textureManager, DrawDataManager* drawDataManager);

	//ファイル名が未記入の場合は、辞書順で一番若いものが選ばれる
	int LoadModel(std::string filePath, std::string fileName = "");
	Animation LoadAnimation(int index, std::string filePath, std::string fileName = "");

	//失敗したらLogとキューブが返る
	NodeModelData& GetNodeModelData(int id);
	//失敗したらLogとキューブが返る
	SkinningModelData& GetSkinningModelData(int id);

private:

	std::string FilePathChecker(std::string& filePath, std::string fileName = "");

	NodeModelData WritingNodeModelData(const aiScene* scene, std::string filePath);
	SkinningModelData WritingSkinningModelData(const aiScene* scene, std::string filePath);

private://Model

	std::unordered_map<std::string, int> modelFilePaths_{};

	std::vector<NodeModelData> nodeModelDatas_{};
	std::vector<SkinningModelData> skinningModelDatas_{};

	int nextID_ = 0;

	TextureManager* textureManager_;
	DrawDataManager* drawDataManager_;


private://Animation

	std::unordered_map<std::string, std::vector<Animation>> animations_{};

private://Debug

	Logger logger_;

};

Matrix4x4 AnimationUpdate(const Animation& animation, float time, const Node& node);
void AnimationUpdate(const Animation& animation, float time, Skeleton& skeleton);
void SkeletonUpdate(Skeleton& skeleton);
void SkinningUpdate(std::vector<WellForGPU>& result, std::map<std::string, JointWeightData> skinCluster, const Skeleton& skeleton);

Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time);
Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time);
