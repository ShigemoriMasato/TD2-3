#include "SamplerTest.h"
#include <imgui/imgui.h>
#include <format>

SamplerTest::~SamplerTest() {
	Save();
}

void SamplerTest::Initialize(ModelManager* modelManager, DrawDataManager* drawDataManager, Camera* camera) {
	camera_ = camera;

	renderObject_ = std::make_unique<RenderObject>();
	renderObject_->Initialize();
	renderObject_->psoConfig_.ps = "PostEffect/Simple.PS.hlsl";
	renderObject_->psoConfig_.vs = "Simple.VS.hlsl";
	renderObject_->SetUseTexture(true);
	renderObject_->CreateCBV(sizeof(VSData), ShaderType::VERTEX_SHADER, "SamplerTest VSData CBV");

	samplerNames_[SamplerID::Default] = "Default";
	samplerNames_[SamplerID::ClampT] = "ClampT";
	samplerNames_[SamplerID::MirrorT] = "MirrorT";
	samplerNames_[SamplerID::ClampS] = "ClampS";
	samplerNames_[SamplerID::MirrorS] = "MirrorS";
	samplerNames_[SamplerID::MagNearest] = "MagNearest";
	samplerNames_[SamplerID::MagLinear] = "MagLinear";
	samplerNames_[SamplerID::MinNearest] = "MinNearest";
	samplerNames_[SamplerID::MinLinear] = "MinLinear";
	samplerNames_[SamplerID::MinNearestMipmapNearest] = "MinNearestMipmapNearest";
	samplerNames_[SamplerID::MinLinearMipmapNearest] = "MinLinearMipmapNearest";
	samplerNames_[SamplerID::MinNearestMipmapLinear] = "MinNearestMipmapLinear";
	samplerNames_[SamplerID::MinLinearMipmapLinear] = "MinLinearMipmapLinear";

	for (int i = 0; i < 14; ++i) {
		std::string num = std::format("{:02}", i);
		modelNames_.push_back("Texture_Sampler_" + num + ".gltf");
	}

	Load();

	SamplerID selectedSampler = SamplerID::Default;
	int i = 0;
	for (const auto& [id, name] : samplerNames_) {
		if(i == currentSamplerIndex_) {
			selectedSampler = id;
			break;
		}
		++i;
	}

	auto model = modelManager->GetNodeModelData(modelManager->LoadModel(directorypath_, modelNames_[currentSamplerIndex_]));
	auto drawData = drawDataManager->GetDrawData(model.drawDataIndices);
	renderObject_->SetDrawData(drawData);

	renderObject_->CreateCBV(sizeof(int), ShaderType::PIXEL_SHADER, "SamplerTest PSData CBV");
	renderObject_->CopyBufferData(1, &model.materials.front().textureIndex, sizeof(int));

	renderObject_->psoConfig_.rootConfig.samplers = uint32_t(selectedSampler);

	modelManager_ = modelManager;
	drawDataManager_ = drawDataManager;
}

void SamplerTest::Draw(Window* window) {
	//VS用データ更新
	vsData_.world = Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.position);
	vsData_.vp = camera_->GetVPMatrix();
	renderObject_->CopyBufferData(0, &vsData_, sizeof(vsData_));
	//描画
	renderObject_->Draw(window);
}

void SamplerTest::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("SamplerTest");

	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.1f, 0.1f, 10.0f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.1f, -360.0f, 360.0f);
	ImGui::DragFloat3("Position", &transform_.position.x, 0.1f, -100.0f, 100.0f);

	ImGui::Separator();

	int index = 0;
	for (const auto& [samplerID, samplerName] : samplerNames_) {
		if (ImGui::RadioButton(samplerName.c_str(), &currentSamplerIndex_, index)) {
			currentSamplerIndex_ = index;
			renderObject_->psoConfig_.rootConfig.samplers = uint32_t(samplerID);
			auto drawData = drawDataManager_->GetDrawData(modelManager_->GetNodeModelData(modelManager_->LoadModel(directorypath_, modelNames_[currentSamplerIndex_])).drawDataIndices);
			renderObject_->SetDrawData(drawData);
		}
		++index;
	}

	ImGui::End();
#endif
}

void SamplerTest::Save() {
	binaryManager_.RegistOutput(transform_.scale);
	binaryManager_.RegistOutput(transform_.rotate);
	binaryManager_.RegistOutput(transform_.position);
	binaryManager_.RegistOutput(currentSamplerIndex_);
	binaryManager_.Write(saveFile_);
}

void SamplerTest::Load() {
	auto values = binaryManager_.Read(saveFile_);
	if (values.empty()) {
		return;
	}
	transform_.scale = BinaryManager::Reverse<Vector3>(values[0]);
	transform_.rotate = BinaryManager::Reverse<Vector3>(values[1]);
	transform_.position = BinaryManager::Reverse<Vector3>(values[2]);
	currentSamplerIndex_ = BinaryManager::Reverse<int>(values[3]);
}
