#include "StageEditor.h"
#include <imgui/imgui.h>
#include <string>

void StageEditor::Initialize() {
	stageData_.resize(1);
	Load();
}

int StageEditor::DrawImGui() {

#ifdef USE_IMGUI

	if (!ImGui::Begin("Stage Editor")) {
		ImGui::End();
		return -1;
	}

	ImGui::PushID(0);
	ImGui::Text("Stage: %d", currentStage_);
	ImGui::SameLine();
	if (ImGui::Button("-")) {
		currentStage_ = std::max(0, currentStage_ - 1);
		currentMapNum_ = 0;
	}
	ImGui::SameLine();
	if (ImGui::Button("+")) {
		currentStage_++;
		currentMapNum_ = 0;
		if(stageData_.size()<=currentStage_){
			stageData_.emplace_back();
			stageData_.back().configs.emplace_back();
		}
	}
	ImGui::PopID();

	if (stageData_.empty()) {
		stageData_.emplace_back();
		stageData_.back().configs.emplace_back();
	}
	StageData& stage = stageData_[currentStage_];

	ImGui::DragInt("初期Ore数", &stage.initOreNum, 1.0f, 0, 1000);
	ImGui::Separator();

	ImGui::PushID(1);
	ImGui::Text("MapNum: %d", int(stage.configs.size()));
	ImGui::SameLine();
	if (ImGui::Button("-")) {
		if (stage.configs.size() > 1) {
			stage.configs.pop_back();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("+")) {
		stage.configs.emplace_back();
	}
	ImGui::PopID();

	for (size_t i = 0; i < stage.configs.size(); ++i) {
		if (ImGui::Button(std::to_string(i).c_str())) {
			currentMapNum_ = static_cast<int>(i);
		}
		if (i != stage.configs.size() - 1) ImGui::SameLine();
	}
	
	MapConfig& config = stage.configs[currentMapNum_];
	
	ImGui::PushID(2);
	ImGui::Text("MapID: %d", config.mapID);
	ImGui::SameLine();
	if (ImGui::Button("-")) {
		config.mapID = std::max(0, config.mapID - 1);
	}
	ImGui::SameLine();
	if (ImGui::Button("+")) {
		config.mapID = std::min(maxMapNum_ - 1, config.mapID + 1);
	}
	ImGui::PopID();

	ImGui::DragInt("ノルマ", &config.norma, 1.0f, 0);

	ImGui::End();

	return config.mapID;

#endif // USE_IMGUI

	return -1;
}

void StageEditor::Save() {

	for (const auto& stage : stageData_) {
		binaryManager_.RegisterOutput(stage.initOreNum);
		int mapSize = static_cast<int>(stage.configs.size());
		binaryManager_.RegisterOutput(mapSize);
		for (const auto& config : stage.configs) {
			binaryManager_.RegisterOutput(config.mapID);
			binaryManager_.RegisterOutput(config.norma);
		}
	}

	binaryManager_.Write(saveFileName_);
}

void StageEditor::Load() {
	auto values = binaryManager_.Read(saveFileName_);
	size_t index = 0;
	stageData_.clear();

	while (index < values.size()) {
		StageData stage;
		stage.initOreNum = BinaryManager::Reverse<int>(values[index++].get());
		int mapSize = BinaryManager::Reverse<int>(values[index++].get());
		for (int i = 0; i < mapSize; ++i) {
			MapConfig config;
			config.mapID = BinaryManager::Reverse<int>(values[index++].get());
			config.norma = BinaryManager::Reverse<int>(values[index++].get());
			stage.configs.push_back(config);
		}
		stageData_.push_back(stage);
	}
}
