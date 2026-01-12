#pragma once
#include "ModelData.h"
#include <assimp/scene.h>
#include <unordered_set>
#include <Assets/Texture/TextureManager.h>

namespace ModelLoader {
	Node ReadNode(const aiNode* node);

	std::vector<Mesh> LoadMeshes(const aiScene* scene);

	std::vector<VertexData> LoadVertices(const aiScene* scene);
	std::vector<VertexInfluence> LoadVertexInfluences(const aiScene* scene);
	std::vector<uint32_t> LoadIndices(const aiScene* scene);

	std::vector<Material> LoadMaterials(const aiScene* scene, std::string directoryPath, TextureManager* textureManager);
	std::vector<uint32_t> LoadMaterialIndices(const aiScene* scene);

	std::map<std::string, JointWeightData> LoadSkinCluster(const aiScene* scene);
	Skeleton CreateSkelton(const Node& rootNode, const aiScene* scene);
	int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent, Skeleton& skeleton, std::unordered_set<std::string>& boneNames, const Matrix4x4 parentAccumulated);

	std::vector<Animation> LoadAnimations(const aiScene* scene);
}
