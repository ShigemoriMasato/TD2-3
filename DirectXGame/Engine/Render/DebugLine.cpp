#include "DebugLine.h"

void DebugLine::Initialize(DrawDataManager* manager, ModelManager* modelManager, Camera* camera) {
	auto cube = manager->GetDrawData(modelManager->GetNodeModelData(0).drawDataIndices);
	auto sphere = manager->GetDrawData(modelManager->GetNodeModelData(2).drawDataIndices);

	sphere_ = std::make_unique<RenderObject>("DebugLine::Sphere");
	sphere_->Initialize();
	sphere_->SetDrawData(sphere);
	sphere_->CreateSRV(sizeof(Matrix4x4), maxDrawNums_, ShaderType::VERTEX_SHADER, "DebugLine::Sphere::World");
	sphere_->CreateCBV(sizeof(Matrix4x4), ShaderType::VERTEX_SHADER, "DebugLine::Sphere::VPMatrix");
	sphere_->psoConfig_.rasterizerID = RasterizerID::Wireframe;
	sphere_->psoConfig_.vs = "ShapeLine.VS.hlsl";
	sphere_->psoConfig_.ps = "White.PS.hlsl";
	sphereVSData_.resize(maxDrawNums_, {});

	cube_ = std::make_unique<RenderObject>("DebugLine::Cube");
	cube_->Initialize();
	cube_->SetDrawData(cube);
	cube_->CreateSRV(sizeof(Matrix4x4), maxDrawNums_, ShaderType::VERTEX_SHADER, "DebugLine::Cube::World");
	cube_->CreateCBV(sizeof(Matrix4x4), ShaderType::VERTEX_SHADER, "DebugLine::Cube::VPMatrix");
	cube_->psoConfig_.rasterizerID = RasterizerID::Wireframe;
	cube_->psoConfig_.vs = "ShapeLine.VS.hlsl";
	cube_->psoConfig_.ps = "White.PS.hlsl";
	cubeVSData_.resize(maxDrawNums_, {});

	lineVertices_.resize(lineNums_ * 2, {});
	for (int i = 0; i < 2; ++i) {
	}
	manager->AddVertexBuffer(lineVertices_);
	drawDataIndex_ = manager->CreateDrawData();
	DrawData DD = manager->GetDrawData(drawDataIndex_);

	line_ = std::make_unique<RenderObject>("DebugLine::Line");
	line_->Initialize();
	line_->SetDrawData(DD);
	line_->CreateCBV(sizeof(Matrix4x4), ShaderType::VERTEX_SHADER, "DebugLine::Line::VPMatrix");
	line_->psoConfig_.topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
	line_->psoConfig_.vs = "Line.VS.hlsl";
	line_->psoConfig_.ps = "White.PS.hlsl";
	line_->psoConfig_.inputLayoutID = InputLayoutID::Vector3;

	drawDataManager_ = manager;
	camera_ = camera;
}

void DebugLine::AddLine(Skeleton& skeleton, float jointScale) {
	for(Joint& joint : skeleton.joints) {
		sphereVSData_[currentSphereNum_++] = Matrix::MakeScaleMatrix({ jointScale, jointScale, jointScale }) * joint.skeletonSpaceMatrix;

		for(const auto& childIndex : joint.children) {
			//線分
			lineVertices_[currentLineNum_++] = Vector3(joint.skeletonSpaceMatrix.m[3][0], joint.skeletonSpaceMatrix.m[3][1], joint.skeletonSpaceMatrix.m[3][2]);
			lineVertices_[currentLineNum_++] = Vector3(skeleton.joints[childIndex].skeletonSpaceMatrix.m[3][0],
				skeleton.joints[childIndex].skeletonSpaceMatrix.m[3][1],
				skeleton.joints[childIndex].skeletonSpaceMatrix.m[3][2]);
		}
	}
}

void DebugLine::Draw(Window* window) {
	Matrix4x4 vpMatrix = camera_->GetVPMatrix();
	if (currentSphereNum_ > 0) {
		sphere_->CopyBufferData(1, &vpMatrix, sizeof(Matrix4x4));
		sphere_->CopyBufferData(0, sphereVSData_.data(), sizeof(Matrix4x4) * currentSphereNum_);
		sphere_->instanceNum_ = currentSphereNum_;
		sphere_->Draw(window);
	}

	if (currentCubeNum_ > 0) {
		cube_->CopyBufferData(1, &vpMatrix, sizeof(Matrix4x4));
		cube_->CopyBufferData(0, cubeVSData_.data(), sizeof(Matrix4x4) * currentCubeNum_);
		cube_->instanceNum_ = currentCubeNum_;
		cube_->Draw(window);
	}

	if (currentLineNum_ > 0) {
		drawDataManager_->CoppyBufferData(drawDataIndex_, lineVertices_.data(), sizeof(Vector3) * currentLineNum_);
		line_->CopyBufferData(0, &vpMatrix, sizeof(Matrix4x4));
		line_->instanceNum_ = currentLineNum_ / 2;
		line_->Draw(window);
	}

	currentSphereNum_ = 0;
	currentCubeNum_ = 0;
	currentLineNum_ = 0;
}

void DebugLine::Fresh() {
	currentSphereNum_ = 0;
	currentCubeNum_ = 0;
	currentLineNum_ = 0;
}
