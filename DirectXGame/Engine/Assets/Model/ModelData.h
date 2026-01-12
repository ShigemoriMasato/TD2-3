#pragma once
#include <Utility/Vector.h>
#include <Utility/Matrix.h>
#include <Utility/Quaternion.h>
#include <Utility/DataStructures.h>
#include <vector>
#include <string>
#include <optional>
#include <map>
#include <unordered_map>

struct Material {
	int textureIndex;
	int normalTextureIndex = -1;
	Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	Vector4 diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 specular = { 1.0f, 1.0f, 1.0f, 1.0f };
	float shininess = 0.0f;
};

struct Node {
	QuaternionTransform transform;
	Matrix4x4 localMatrix;
	std::string name;
	int drawDataIndex = -1;
	int meshIndex = -1;
	std::vector<Node> children;
};

template <typename T>
struct Keyframe {
	float time;
	T value;
};
using KeyframeVector3 = Keyframe<Vector3>;
using KeyframeQuaternion = Keyframe<Quaternion>;

template<typename T>
struct AnimationCurve {
	std::vector<Keyframe<T>> keyframes;
};

struct NodeAnimation {
	AnimationCurve<Vector3> position;
	AnimationCurve<Quaternion> rotate;
	AnimationCurve<Vector3> scale;
};

struct Animation {
	float duration;
	std::unordered_map<std::string, NodeAnimation> nodeAnimations;
};

struct Mesh {
	int drawDataIndex = -1;
	int materialIndex = -1;
	std::vector<VertexData> vertices{};
	std::vector<uint32_t> indices{};
};

struct NodeModelData {
	std::vector<Mesh> meshes{};

	std::vector<Material> materials{};

	Node rootNode{};
	std::vector<int> drawDataIndices{};
};

struct Joint {
	QuaternionTransform transform;
	Matrix4x4 localMatrix;
	Matrix4x4 skeletonSpaceMatrix;
	std::string name;
	std::vector<int32_t> children;
	int32_t index;
	std::optional<int32_t> parent;
};

struct Skeleton {
	int32_t root;
	Matrix4x4 rootMatrix = Matrix4x4::Identity();
	std::map<std::string, int32_t> jointMap;
	std::vector<Joint> joints;
};

struct VertexWeightData {
	float weight;
	uint32_t vertexIndex;
};

struct JointWeightData {
	Matrix4x4 inverseBindPoseMatrix{};
	std::vector<VertexWeightData> vertexWeights;
};

const uint32_t kMaxInfluences = 4;
struct VertexInfluence {
	float weights[kMaxInfluences];
	int jointIndices[kMaxInfluences];
};

struct WellForGPU {
	Matrix4x4 skeletonSpaceMatrix;//位置用
	Matrix4x4 skeletonSpaceInverseTransposeMatrix;//normal用
};

struct SkinningModelData {
	//VertexBufferView関連
	std::vector<VertexData> vertices{};
	std::vector<VertexInfluence> vertexInfluences{};
	std::vector<uint32_t> materialIndex{};

	//IndexBufferView関連
	std::vector<uint32_t> indices{};

	//その他
	Skeleton skeleton{};
	std::map<std::string, JointWeightData> skinClusterData{};
	std::vector<Material> materials{};
	int drawDataIndex = -1;
};
