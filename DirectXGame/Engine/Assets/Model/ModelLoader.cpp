#include "ModelLoader.h"
#include <Utility/MatrixFactory.h>
#include <Utility/Color.h>

Node ModelLoader::ReadNode(const aiNode* node) {
	Node result;

	//Transformの読み込み
	aiVector3D scale, position;
	aiQuaternion rotate;
	node->mTransformation.Decompose(scale, rotate, position);
	result.transform.scale = { scale.x, scale.y, scale.z };
	result.transform.rotate = { rotate.x, rotate.y, rotate.z, rotate.w };
	result.transform.position = { position.x, position.y, position.z };

	//ローカル行列の計算
	result.localMatrix = 
		Matrix::MakeScaleMatrix(result.transform.scale) * 
		result.transform.rotate.ToMatrix() * 
		Matrix::MakeTranslationMatrix(result.transform.position);

	// Node名を格納
	result.name = node->mName.C_Str();

	//メッシュインデックスの格納（複数ある場合は最初のものを使う）
	result.meshIndex = (node->mNumMeshes > 0) ? node->mMeshes[0] : -1;

	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		//再帰的に読んで階層構造を作っていく
		result.children.push_back(ReadNode(node->mChildren[childIndex]));
	}

	return result;
}

std::vector<Mesh> ModelLoader::LoadMeshes(const aiScene* scene) {
	std::vector<Mesh> meshes;
	for (uint32_t mesh = 0; mesh < scene->mNumMeshes; ++mesh) {
		aiMesh* ai_mesh = scene->mMeshes[mesh];
		Mesh meshData{};
		//頂点データの読み込み
		for (uint32_t v = 0; v < ai_mesh->mNumVertices; ++v) {
			VertexData vertex{};
			//位置
			vertex.position.x = ai_mesh->mVertices[v].x;
			vertex.position.y = ai_mesh->mVertices[v].y;
			vertex.position.z = ai_mesh->mVertices[v].z;
			vertex.position.w = 1.0f;
			//法線
			if (ai_mesh->HasNormals()) {
				vertex.normal.x = ai_mesh->mNormals[v].x;
				vertex.normal.y = ai_mesh->mNormals[v].y;
				vertex.normal.z = ai_mesh->mNormals[v].z;
			}
			//UV
			if (ai_mesh->HasTextureCoords(0)) {
				vertex.texcoord.x = ai_mesh->mTextureCoords[0][v].x;
				vertex.texcoord.y = ai_mesh->mTextureCoords[0][v].y;
			}
			meshData.vertices.push_back(vertex);
		}
		//インデックスデータの読み込み
		for (uint32_t f = 0; f < ai_mesh->mNumFaces; ++f) {
			aiFace& face = ai_mesh->mFaces[f];
			for (uint32_t i = 0; i < face.mNumIndices; ++i) {
				meshData.indices.push_back(face.mIndices[i]);
			}
		}
		meshData.materialIndex = ai_mesh->mMaterialIndex;
		meshes.push_back(meshData);
	}

	return meshes;
}

std::vector<VertexData> ModelLoader::LoadVertices(const aiScene* scene) {
	std::vector<VertexData> vertices;

	for(uint32_t node = 0; node < scene->mNumMeshes; ++node) {
		aiMesh* ai_mesh = scene->mMeshes[node];

		for (uint32_t v = 0; v < ai_mesh->mNumVertices; ++v) {
			VertexData vertex{};
			//位置
			vertex.position.x = ai_mesh->mVertices[v].x;
			vertex.position.y = ai_mesh->mVertices[v].y;
			vertex.position.z = ai_mesh->mVertices[v].z;
			vertex.position.w = 1.0f;
			//法線
			if (ai_mesh->HasNormals()) {
				vertex.normal.x = ai_mesh->mNormals[v].x;
				vertex.normal.y = ai_mesh->mNormals[v].y;
				vertex.normal.z = ai_mesh->mNormals[v].z;
			}
			//UV
			if (ai_mesh->HasTextureCoords(0)) {
				vertex.texcoord.x = ai_mesh->mTextureCoords[0][v].x;
				vertex.texcoord.y = ai_mesh->mTextureCoords[0][v].y;
			}
			vertices.push_back(vertex);
		}
	}

	return vertices;
}

std::vector<VertexInfluence> ModelLoader::LoadVertexInfluences(const aiScene* scene) {
	std::vector<VertexInfluence> vertexInfluences;
	//頂点数分の空のデータを作成
	uint32_t totalVertexCount = 0;
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		totalVertexCount += mesh->mNumVertices;
	}
	vertexInfluences.resize(totalVertexCount);
	//各メッシュのボーン情報を読み込み
	uint32_t vertexOffset = 0;
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			aiBone* bone = mesh->mBones[boneIndex];
			for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
				aiVertexWeight weight = bone->mWeights[weightIndex];
				VertexInfluence& vertexInfluence = vertexInfluences[vertexOffset + weight.mVertexId];
				//影響度を追加
				for (int i = 0; i < kMaxInfluences; ++i) {
					if (vertexInfluence.weights[i] == 0.0f) {
						vertexInfluence.weights[i] = weight.mWeight;
						vertexInfluence.jointIndices[i] = boneIndex; //ボーンのインデックスを格納
						break;
					}
				}
			}
		}
		vertexOffset += mesh->mNumVertices;
	}
	return vertexInfluences;
}

std::vector<uint32_t> ModelLoader::LoadIndices(const aiScene* scene) {
	std::vector<uint32_t> indices;

	for (uint32_t mesh = 0; mesh < scene->mNumMeshes; ++mesh) {
		aiMesh* ai_mesh = scene->mMeshes[mesh];

		for (uint32_t f = 0; f < ai_mesh->mNumFaces; ++f) {
			aiFace& face = ai_mesh->mFaces[f];

			for (uint32_t i = 0; i < face.mNumIndices; ++i) {
				indices.push_back(face.mIndices[i]);
			}
		}
	}

	return indices;
}

std::vector<Material> ModelLoader::LoadMaterials(const aiScene* scene, std::string directoryPath, TextureManager* textureManager) {
	std::vector<Material> materials;

	for (uint32_t i = 0; i < scene->mNumMaterials; ++i) {
		aiMaterial* ai_material = scene->mMaterials[i];
		Material material{};

		//テクスチャ読み込み
		aiString texturePath;
		if (ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
			std::string path = texturePath.C_Str();
			material.textureIndex = textureManager->LoadTexture(directoryPath + "/" + path);
		} else {
			material.textureIndex = 0; //テクスチャが無い場合はデフォルトテクスチャを使う
		}

		aiString normalTexturePath;
		if (ai_material->GetTexture(aiTextureType_NORMALS, 0, &normalTexturePath) == AI_SUCCESS) {
			std::string path = normalTexturePath.C_Str();
			material.normalTextureIndex = textureManager->LoadTexture(directoryPath + "/" + path);
		} else {
			material.normalTextureIndex = -1; //法線マップが無い場合
		}

		aiColor4D baseColor{};
		if (ai_material->Get(AI_MATKEY_BASE_COLOR, baseColor) == AI_SUCCESS) {
			material.color = { baseColor.r, baseColor.g, baseColor.b, baseColor.a };
		}

		aiColor4D diffuseColor{};
		if (ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
			material.color = { diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a };
		}

		aiColor4D specularColor{};
		if (ai_material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS) {
			material.specular = { specularColor.r, specularColor.g, specularColor.b, specularColor.a };
		}

		float shininess = 0.f;
		if (ai_material->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
			material.shininess = shininess;
		}

		materials.push_back(material);
	}

	return materials;
}

std::vector<uint32_t> ModelLoader::LoadMaterialIndices(const aiScene* scene) {
	std::vector<uint32_t> result;

	for (uint32_t mesh = 0; mesh < scene->mNumMeshes; ++mesh) {
		aiMesh* ai_mesh = scene->mMeshes[mesh];

		for (uint32_t f = 0; f < ai_mesh->mNumFaces; ++f) {
			for (uint32_t i = 0; i < ai_mesh->mFaces[f].mNumIndices; ++i) {
				result.push_back(ai_mesh->mMaterialIndex);
			}
		}
	}

	return result;
}

std::map<std::string, JointWeightData> ModelLoader::LoadSkinCluster(const aiScene* scene) {
	std::map<std::string, JointWeightData> skinClusterData;

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];

		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			aiBone* bone = mesh->mBones[boneIndex];
			std::string jointName = bone->mName.C_Str();
			JointWeightData& jointWeightData = skinClusterData[jointName];

			aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
			aiVector3D scale, translate;
			aiQuaternion rotate;
			bindPoseMatrixAssimp.Decompose(scale, rotate, translate);
			Matrix4x4 bindPoseMatrix =
				Matrix::MakeScaleMatrix({ scale.x, scale.y, scale.z }) *
				Quaternion(rotate.x, rotate.y, rotate.z, rotate.w).ToMatrix() *
				Matrix::MakeTranslationMatrix({ translate.x, translate.y, translate.z });
			jointWeightData.inverseBindPoseMatrix = bindPoseMatrix.Inverse();

			for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
				jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight, bone->mWeights[weightIndex].mVertexId });
			}
		}
	}

	return skinClusterData;
}

Skeleton ModelLoader::CreateSkelton(const Node& rootNode, const aiScene* scene) {
	Skeleton skeleton{};
	std::unordered_set<std::string> boneNames;

	for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
		for (uint32_t b = 0; b < scene->mMeshes[i]->mNumBones; ++b) {
			boneNames.insert(scene->mMeshes[i]->mBones[b]->mName.C_Str());
		}
	}
	skeleton.root = CreateJoint(rootNode, {}, skeleton, boneNames, Matrix4x4::Identity());

	//名前とindexのマッピング
	for (const Joint& joint : skeleton.joints) {
		skeleton.jointMap[joint.name] = joint.index;
	}

	return skeleton;
}

int32_t ModelLoader::CreateJoint(const Node& node, const std::optional<int32_t>& parent, Skeleton& skeleton, std::unordered_set<std::string>& boneNames, Matrix4x4 parentAccumulated) {
	Matrix4x4 currentAccumulated = parentAccumulated * node.localMatrix;

	auto& joints = skeleton.joints;
	if (!boneNames.contains(node.name)) {
		// Jointにしないが、子は見る
		for (const Node& child : node.children) {
			skeleton.rootMatrix = node.localMatrix * skeleton.rootMatrix;
			CreateJoint(child, parent, skeleton, boneNames, currentAccumulated);
		}
		return -1;
	}

	Joint joint{};
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = Matrix4x4::Identity();
	joint.transform = node.transform;
	joint.index = static_cast<int32_t>(joints.size());
	joint.parent = parent;

	joints.push_back(joint);

	if(!parent.has_value()) {
		skeleton.rootMatrix = parentAccumulated;
	}

	for (const Node& child : node.children) {
		int32_t childIndex = CreateJoint(child, joint.index, skeleton, boneNames, Matrix4x4::Identity());
		joints[joint.index].children.push_back(childIndex);
	}

	return joint.index;
}

std::vector<Animation> ModelLoader::LoadAnimations(const aiScene* scene) {
	std::vector<Animation> animations{};

	if(scene->mNumAnimations == 0) {
		return animations;
	}

	for (uint32_t i = 0; i < scene->mNumAnimations; ++i) {
		Animation animation{};
		aiAnimation* ai_animation = scene->mAnimations[i];

		animation.duration = static_cast<float>(ai_animation->mDuration / ai_animation->mTicksPerSecond);

		for (uint32_t channelIndex = 0; channelIndex < ai_animation->mNumChannels; ++channelIndex) {
			aiNodeAnim* ai_nodeAnim = ai_animation->mChannels[channelIndex];
			NodeAnimation nodeAnimation{};
			//位置
			for (uint32_t posIndex = 0; posIndex < ai_nodeAnim->mNumPositionKeys; ++posIndex) {
				KeyframeVector3 keyframe{};
				keyframe.time = static_cast<float>(ai_nodeAnim->mPositionKeys[posIndex].mTime / ai_animation->mTicksPerSecond);
				keyframe.value.x = ai_nodeAnim->mPositionKeys[posIndex].mValue.x;
				keyframe.value.y = ai_nodeAnim->mPositionKeys[posIndex].mValue.y;
				keyframe.value.z = ai_nodeAnim->mPositionKeys[posIndex].mValue.z;
				nodeAnimation.position.keyframes.push_back(keyframe);
			}
			//回転
			for (uint32_t rotIndex = 0; rotIndex < ai_nodeAnim->mNumRotationKeys; ++rotIndex) {
				KeyframeQuaternion keyframe{};
				keyframe.time = static_cast<float>(ai_nodeAnim->mRotationKeys[rotIndex].mTime / ai_animation->mTicksPerSecond);
				keyframe.value.x = ai_nodeAnim->mRotationKeys[rotIndex].mValue.x;
				keyframe.value.y = ai_nodeAnim->mRotationKeys[rotIndex].mValue.y;
				keyframe.value.z = ai_nodeAnim->mRotationKeys[rotIndex].mValue.z;
				keyframe.value.w = ai_nodeAnim->mRotationKeys[rotIndex].mValue.w;
				nodeAnimation.rotate.keyframes.push_back(keyframe);
			}
			//スケール
			for (uint32_t scaleIndex = 0; scaleIndex < ai_nodeAnim->mNumScalingKeys; ++scaleIndex) {
				KeyframeVector3 keyframe{};
				keyframe.time = static_cast<float>(ai_nodeAnim->mScalingKeys[scaleIndex].mTime / ai_animation->mTicksPerSecond);
				keyframe.value.x = ai_nodeAnim->mScalingKeys[scaleIndex].mValue.x;
				keyframe.value.y = ai_nodeAnim->mScalingKeys[scaleIndex].mValue.y;
				keyframe.value.z = ai_nodeAnim->mScalingKeys[scaleIndex].mValue.z;
				nodeAnimation.scale.keyframes.push_back(keyframe);
			}

			animation.nodeAnimations[ai_nodeAnim->mNodeName.C_Str()] = nodeAnimation;
		}

		animations.push_back(animation);
	}

	return animations;
}
