#include "ModelManager.h"

#include <Utility/SearchFile.h>
#include <Utility/Easing.h>
#include <Utility/MatrixFactory.h>
#include "ModelLoader.h"

#include <cassert>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace {
	std::vector<std::string> ExtensionSearcher(std::string directoryPath, std::vector<std::string> exts) {
		std::vector<std::string> result;
		result.reserve(64); // 目安。分かるならもっと正確に

		for (const auto& ext : exts) {
			auto files = SearchFiles(directoryPath, ext);
			result.insert(result.end(), std::make_move_iterator(files.begin()), std::make_move_iterator(files.end()));
		}

		return result;
	}
}

void ModelManager::Initialize(TextureManager* textureManager, DrawDataManager* drawDataManager) {
	modelFilePaths_.clear();
	nodeModelDatas_.clear();
	skinningModelDatas_.clear();
	textureManager_ = textureManager;
	drawDataManager_ = drawDataManager;
	nextID_ = 0;

	logger_ = getLogger("Engine");

	LoadModel("Assets/.EngineResource/Model/Cube");
	LoadModel("Assets/.EngineResource/Model/Plane");
	LoadModel("Assets/.EngineResource/Model/Sphere");
	LoadModel("Assets/.EngineResource/Model/SimpleSkin");
	LoadModel("Assets/.EngineResource/Model/DefaultDesc");
}

int ModelManager::LoadModel(std::string filePath, std::string fileName) {
	// ファイルパスの確認と修正
	std::string factName = FilePathChecker(filePath, fileName);
	std::string path = (filePath + "/" + factName);

	// すでに読み込んでいたらIDを返す
	const auto it = modelFilePaths_.find(path);
	if (it != modelFilePaths_.end()) {
		logger_->debug("Model already loaded: {}", path);
		return it->second;
	}

	logger_->info("Loading Model: {}", path);

	//idの設定
	int id = -1;

	//Assimp
	Assimp::Importer importer;
	const aiScene* scene = nullptr;
	scene = importer.ReadFile(path.c_str(), aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate);
	assert(scene && "ModelManager::LoadModel: Failed to load model");

	//読み込み

	bool isSkinningModel = false;
	for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
		aiMesh* mesh = scene->mMeshes[m];
		if (mesh->HasBones()) {
			isSkinningModel = true;
			break;
		}
	}

	if (isSkinningModel) {
		SkinningModelData result = WritingSkinningModelData(scene, filePath);

		id = int(skinningModelDatas_.size());
		skinningModelDatas_.push_back(result);
	} else {
		NodeModelData result = WritingNodeModelData(scene, filePath);

		id = int(nodeModelDatas_.size());
		nodeModelDatas_.push_back(result);
	}

	modelFilePaths_[path] = id;

	return id;
}

Animation ModelManager::LoadAnimation(int index, std::string filePath, std::string fileName) {

	std::string factName = FilePathChecker(filePath, fileName);
	std::string path = (filePath + "/" + factName);

	auto it = animations_.find(path);
	if (it != animations_.end()) {
		logger_->debug("Animation already loaded: {}", path);

		if (it->second.empty()) {
			logger_->error("No animations found in file: {}", path);
			assert(false && "ModelManager::LoadAnimation: No animations found");
			return Animation{};
		}

		index = std::clamp(index, 0, int(it->second.size() - 1));
		return it->second[index];
	}

	//Assimp
	Assimp::Importer importer;
	const aiScene* scene = nullptr;
	scene = importer.ReadFile(path.c_str(), aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate);
	assert(scene && "ModelManager::LoadModel: Failed to load model");

	//読み込み
	auto animations = ModelLoader::LoadAnimations(scene);

	if (animations.empty()) {
		logger_->error("No animations found in file: {}", path);
		assert(false && "ModelManager::LoadAnimation: No animations found");
		return Animation{};
	}

	animations_[path] = animations;
	index = std::clamp(index, 0, int(animations.size() - 1));
	return animations_[path][index];
}

NodeModelData& ModelManager::GetNodeModelData(int id) {
	if (nodeModelDatas_.size() > id) {
		return nodeModelDatas_[id];
	} else {
		logger_->error("Model ID not found: {}", id);
		assert(false && "ModelManager::GetNodeModelData: Model ID not found");
		return nodeModelDatas_[0]; //キューブをセット
	}
}

SkinningModelData& ModelManager::GetSkinningModelData(int id) {
	if (skinningModelDatas_.size() > id) {
		return skinningModelDatas_[id];
	} else {
		logger_->error("Model ID not found: {}", id);
		assert(false && "ModelManager::GetSkinningModelData: Model ID not found");
		return skinningModelDatas_[0]; //キューブをセット
	}
}

std::string ModelManager::FilePathChecker(std::string& filePath, std::string fileName) {
	//Assets/から始まっているか確認(Assets/Modelの可能性もあるのでAssets/のみ確認)
	std::string formatFirst = "Assets/";
	std::string factFilePath = "";
	if (filePath.length() < formatFirst.length()) {
		factFilePath = "Assets/Model/" + filePath;
	} else {
		for (int i = 0; i < formatFirst.length(); ++i) {
			if (filePath[i] != formatFirst[i]) {
				factFilePath = "Assets/Model/" + filePath;
				break;
			}

			if (i == formatFirst.length() - 1) {
				factFilePath = filePath;
			}
		}
	}
	filePath = factFilePath;

	std::vector<std::string> extensions = { ".fbx", ".obj", ".gltf", ".glb" };
	std::vector<std::string> fileNames = ExtensionSearcher(filePath, extensions);

	if (fileNames.empty()) {
		return "";
	}

	if (fileName == "") {
		std::sort(fileNames.begin(), fileNames.end());
		return fileNames.front();
	} else {
		for (const auto& name : fileNames) {
			if (name == fileName) {
				return fileName;
			}
		}
		logger_->warn("File not found: {}, so using {} instead.", fileName, fileNames.front());
		return fileNames.front();
	}

	return fileName;
}

NodeModelData ModelManager::WritingNodeModelData(const aiScene* scene, std::string filePath) {
	NodeModelData result;
	//コードがごちゃつくのでModelLoaderに処理を投げる
	result.meshes = ModelLoader::LoadMeshes(scene);
	result.materials = ModelLoader::LoadMaterials(scene, filePath, textureManager_);
	result.rootNode = ModelLoader::ReadNode(scene->mRootNode);

	//読み込めているかの確認
	bool isCorrect = true;
	if (result.meshes.empty()) {
		logger_->warn("Mesh is Empty!");
		isCorrect = false;
	}
	if (result.materials.empty()) {
		logger_->warn("Material is Empty!");
		isCorrect = false;
	}

	if (!isCorrect) {
		logger_->error("Failed to Load Model, So setting Cube instead.");
		assert(false && "ModelManager::LoadModel: Failed to load model");
		result = nodeModelDatas_[0]; //キューブをセット
	} else {
		//DrawDataの作成
		for (const auto& mesh : result.meshes) {
			if (mesh.indices.empty()) {
				continue;
			}
			drawDataManager_->AddVertexBuffer(mesh.vertices);
			drawDataManager_->AddIndexBuffer(mesh.indices);
			int drawDataIndex = drawDataManager_->CreateDrawData();
			result.drawDataIndices.push_back(drawDataIndex);
		}
	}

	return result;
}

SkinningModelData ModelManager::WritingSkinningModelData(const aiScene* scene, std::string filePath) {
	SkinningModelData result;
	//コードがごちゃつくのでModelLoaderに処理を投げる
	result.vertices = ModelLoader::LoadVertices(scene);
	result.vertexInfluences = ModelLoader::LoadVertexInfluences(scene);
	result.indices = ModelLoader::LoadIndices(scene);
	result.materials = ModelLoader::LoadMaterials(scene, filePath, textureManager_);
	result.materialIndex = ModelLoader::LoadMaterialIndices(scene);
	result.skeleton = ModelLoader::CreateSkelton(ModelLoader::ReadNode(scene->mRootNode), scene);
	result.skinClusterData = ModelLoader::LoadSkinCluster(scene);

	//読み込めているかの確認
	bool isCorrect = true;
	if (result.vertices.empty()) {
		logger_->warn("Vertex is Empty!");
		isCorrect = false;
	}
	if (result.vertices.empty()) {
		logger_->warn("VertexInfluence is Empty!");
		isCorrect = false;
	}
	if (result.indices.empty()) {
		logger_->warn("Index is Empty!");
		isCorrect = false;
	}
	if (result.materials.empty()) {
		logger_->warn("Material is Empty!");
		isCorrect = false;
	}
	if (result.materialIndex.empty()) {
		logger_->warn("MaterialIndex is Empty!");
		isCorrect = false;
	}
	if (result.skeleton.joints.empty()) {
		logger_->warn("Skeleton is Empty!");
		isCorrect = false;
	}

	if (!isCorrect) {
		logger_->error("Failed to Load Model, So setting Cube instead.");
		assert(false && "ModelManager::LoadModel: Failed to load model");
		result = skinningModelDatas_[1]; //キューブをセット
	} else {
		//DrawDataの作成
		drawDataManager_->AddVertexBuffer(result.vertices);
		drawDataManager_->AddVertexBuffer(result.vertexInfluences);
		drawDataManager_->AddIndexBuffer(result.indices);
		int drawDataIndex = drawDataManager_->CreateDrawData();
		result.drawDataIndex = drawDataIndex;
	}

	return result;

}

Matrix4x4 AnimationUpdate(const Animation& animation, float time, const Node& node) {
	Vector3 position = CalculateValue(animation.nodeAnimations.at(node.name).position.keyframes, time);
	Quaternion rotation = CalculateValue(animation.nodeAnimations.at(node.name).rotate.keyframes, time);
	Vector3 scale = CalculateValue(animation.nodeAnimations.at(node.name).scale.keyframes, time);
	return Matrix::MakeScaleMatrix(scale) * rotation.ToMatrix() * Matrix::MakeTranslationMatrix(position);
}

void AnimationUpdate(const Animation& animation, float time, Skeleton& skeleton) {
	for (Joint& joint : skeleton.joints) {
		if (auto it = animation.nodeAnimations.find(joint.name); it != animation.nodeAnimations.end()) {
			Vector3 position = CalculateValue(it->second.position.keyframes, time);
			Quaternion rotation = CalculateValue(it->second.rotate.keyframes, time);
			Vector3 scale = CalculateValue(it->second.scale.keyframes, time);
			joint.transform.position = position;
			joint.transform.rotate = rotation;
			joint.transform.scale = scale;
		}
	}
}

void SkeletonUpdate(Skeleton& skeleton) {
	for (Joint& joint : skeleton.joints) {
		joint.localMatrix = Matrix::MakeScaleMatrix(joint.transform.scale) *
			joint.transform.rotate.ToMatrix() *
			Matrix::MakeTranslationMatrix(joint.transform.position);

		if (joint.parent) {
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton.joints[*joint.parent].skeletonSpaceMatrix;
		} else {
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton.rootMatrix;
		}
	}
}

void SkinningUpdate(std::vector<WellForGPU>& result, std::map<std::string, JointWeightData> skinCluster, const Skeleton& skeleton) {
	for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex) {
		assert(jointIndex < skeleton.joints.size());
		std::string key = skeleton.joints[jointIndex].name;
		result[jointIndex].skeletonSpaceMatrix =
			skinCluster[key].inverseBindPoseMatrix * skeleton.joints[jointIndex].skeletonSpaceMatrix;

		result[jointIndex].skeletonSpaceInverseTransposeMatrix =
			Matrix::TransMatrix(result[jointIndex].skeletonSpaceMatrix.Inverse());
	}
}

Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time) {
	assert(!keyframes.empty());
	if (keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}

	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;

		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return lerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}

	return keyframes.back().value;
}

Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time) {
	assert(!keyframes.empty());
	if (keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}

	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;

		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return lerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}

	return keyframes.back().value;
}
