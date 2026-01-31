#include "MapTextureEditor.h"
#include <GameCamera/DebugMousePos.h>
#include <Utility/SearchFile.h>

void MapTextureEditor::Initialize(TextureManager* textureManager, Input* input) {
	assert(textureManager);
	assert(input);
	textureManager_ = textureManager;
	input_ = input;

	TextureLoad();
	MapLoad();
	SetCurrentStage(0);
}

void MapTextureEditor::Update() {
	someSelected_ = false;

	//resize
	fieldData_.resize(height_);
	for (auto& row : fieldData_) {
		row.resize(width_);
	}
	saveData_[currentStage_].resize(height_);
	for (auto& row : saveData_[currentStage_]) {
		row.resize(width_);
	}

	if (!editing_) {
		return;
	}

	//クリックされたら変更
	if (Input::GetMouseButtonState()[0] && DebugMousePos::isHovered) {

		std::pair<int, int> gridedPos = {
			static_cast<int>(cursorPos_.x),
			static_cast<int>(cursorPos_.y)
		};

		if (fieldData_.size() <= gridedPos.second || fieldData_.at(0).size() <= gridedPos.first) {
			return;
		}

		//saveDataにはidを保存
		saveData_[currentStage_][gridedPos.second][gridedPos.first].textureIndex = currentTextureID_;
		saveData_[currentStage_][gridedPos.second][gridedPos.first].direction = currentDirection_;

		//fieldにはindexを保存
		int index = textureManager_->GetTexture("MapChip/" + textureIndexList_[currentTextureID_]);
		fieldData_[gridedPos.second][gridedPos.first].textureIndex = index;
		fieldData_[gridedPos.second][gridedPos.first].direction = currentDirection_;
	}
}

void MapTextureEditor::DrawImGui() {
#ifdef USE_IMGUI

	if (ImGui::Begin("MapTexture")) {
		if (!editing_) {
			editing_ = true;
			someSelected_ = true;
		}
	}

	ImGui::BeginChild("TextureList", ImVec2(600, 0));

	int buttonNum = 0;
	for (const auto& [id, path] : textureIndexList_) {
		++buttonNum;
		int index = textureManager_->GetTexture("MapChip/" + path);

		ImGui::PushID(id);

		ImVec2 size = ImVec2(50, 50);
		if (id == currentTextureID_) {
			size = ImVec2(60, 60);
		}

		//ボタンの描画
		TextureData* textureData = textureManager_->GetTextureData(index);
		if (ImGui::ImageButton(path.c_str(), ImTextureRef(textureData->GetGPUHandle().ptr), size)) {
			//テクスチャをいじっていると示すフラグを立てる
			someSelected_ = true;
			editing_ = true;

			//選択しているテクスチャIDを更新
			currentTextureID_ = id;

			//Config系の初期化
			currentDirection_ = Direction::Front;
			size_t len = path.length() + 1; // +1 for null terminator
			std::memcpy(inputText_, path.c_str(), len);
		}

		ImGui::PopID();

		if (buttonNum % 6 != 0) {
			ImGui::SameLine();
		}
	}

	ImGui::EndChild();

	ImGui::SameLine();

	const float previewSize = 128.0f;
	ImGui::BeginChild("Preview", ImVec2(previewSize, 0));

	TextureData* currentTextureData = textureManager_->GetTextureData(
		textureManager_->GetTexture("MapChip/" + textureIndexList_[currentTextureID_])
	);
	ImVec2 uvs[4];
	switch (currentDirection_) {
	case Direction::Front:
		uvs[0] = { 0.0f, 0.0f };
		uvs[1] = { 1.0f, 0.0f };
		uvs[2] = { 1.0f, 1.0f };
		uvs[3] = { 0.0f, 1.0f };
		break;
	case Direction::Right:
		uvs[0] = { 0.0f, 1.0f }; // LT
		uvs[1] = { 0.0f, 0.0f };// RT
		uvs[2] = { 1.0f, 0.0f }; // RB
		uvs[3] = { 1.0f, 1.0f };  // LB
		break;
	case Direction::Back:
		uvs[0] = { 1.0f, 1.0f }; // LT
		uvs[1] = { 0.0f, 1.0f }; // RT
		uvs[2] = { 0.0f, 0.0f }; // RB
		uvs[3] = { 1.0f, 0.0f }; // LB
		break;
	case Direction::Left:
		uvs[0] = { 1.0f, 0.0f }; // LT
		uvs[1] = { 1.0f, 1.0f }; // RT
		uvs[2] = { 0.0f, 1.0f }; // RB
		uvs[3] = { 0.0f, 0.0f }; // LB
		break;
	}
	ImVec2 min = ImGui::GetCursorScreenPos();
	ImVec2 pos[4] = {
		min,
		{min.x + previewSize, min.y},
		{min.x + previewSize, min.y + previewSize},
		{min.x, min.y + previewSize}
	};
	auto dl = ImGui::GetWindowDrawList();
	dl->AddImageQuad(ImTextureRef(currentTextureData->GetGPUHandle().ptr),
		pos[0], pos[1], pos[2], pos[3], uvs[0], uvs[1], uvs[2], uvs[3]);

	ImGui::Text("Current Texture ID: %s", textureIndexList_[currentTextureID_]);

	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("Config", ImVec2(0, 0));

	ImGui::InputText("|", inputText_, 256);
	if (ImGui::Button("Edit")) {
		int handle = textureManager_->GetTexture("MapChip/" + std::string(inputText_));

		//読み込めない場合は元に戻す
		if (handle == textureManager_->GetErrorTextureHandle()) {
			size_t len = textureIndexList_[currentTextureID_].length() + 1; // +1 for null terminator
			std::memcpy(inputText_, textureIndexList_[currentTextureID_].c_str(), len);
		} else {
			//読み込めたら
			textureIndexList_[currentTextureID_] = std::string(inputText_);

			//フィールドデータを書き換え
			for (size_t i = 0; i < fieldData_.size(); ++i) {
				for (size_t j = 0; j < fieldData_[i].size(); ++j) {
					if (saveData_[currentStage_][i][j].textureIndex == currentTextureID_) {
						fieldData_[i][j].textureIndex = handle;
					}
				}
			}

		}
	}

	ImGui::EndChild();

	ImGui::End();

#endif // USE_IMGUI

}

void MapTextureEditor::SetCurrentStage(int currentStage) {
	currentStage_ = currentStage;

	if (saveData_.size() <= currentStage_) {
		saveData_.resize(currentStage_ + 1);
	}

	height_ = int(saveData_[currentStage_].size());
	width_ = 0;

	fieldData_.resize(height_);
	//stageのデータをfieldDataに反映
	auto& stage = saveData_[currentStage_];
	for (int i = 0; i < stage.size(); ++i) {
		width_ = int(stage[i].size());
		fieldData_[i].resize(width_);
		for (int j = 0; j < stage[i].size(); ++j) {
			//idからfilePathを取得
			int textureID = stage[i][j].textureIndex;
			std::string texturePath = textureIndexList_[textureID];

			//登録されていないidの場合はerrorテクスチャを設定
			if (!textureIndexList_.contains(textureID)) {
				fieldData_[i][j].textureIndex = textureManager_->GetErrorTextureHandle();
				continue;
			}

			//filePathからindexを取得
			int index = textureManager_->GetTexture("MapChip/" + texturePath);

			//indexを挿入(存在しない場合はtextureManagerがerrorを返してくれるのでそのまま入れる)
			fieldData_[i][j].textureIndex = index;
		}
	}
}

void MapTextureEditor::TextureLoad() {
	//Binaryにあるものとフォルダにあるテクスチャを混ぜてどっちも登録する
	//バイナリにあるやつ
	auto values = binaryManager_.Read(saveFileNameTexturePath_);
	//フォルダにあるやつ
	auto files = SearchFiles("Assets/Texture/MapChip/", ".png");
	uint32_t oldestID = 0;

	for (int i = 0; i < values.size(); i) {
		int id = BinaryManager::Reverse<int>(values[i++].get());
		std::string path = BinaryManager::Reverse<std::string>(values[i++].get());
		textureIndexList_[id] = path;

		//Initializeで読み込むためLoadする
		textureManager_->LoadTexture("MapChip/" + path);

		//一番大きいIDを記録しておく
		oldestID = std::max(oldestID, static_cast<uint32_t>(id));

		//重複を避けるためにフォルダ内のリストから削除
		for (int i = 0; i < files.size(); i++) {
			if (files[i] == path) {
				files.erase(files.begin() + i);
				break;
			}
		}
	}

	//フォルダにあるやつで残ったものを登録
	for(const auto& path : files) {
		textureIndexList_[++oldestID] = path;
		textureManager_->LoadTexture("MapChip/" + path);
	}
}

void MapTextureEditor::TextureSave() {
	for (const auto& [id, path] : textureIndexList_) {
		binaryManager_.RegisterOutput(id);
		binaryManager_.RegisterOutput(path);
	}
	binaryManager_.Write(saveFileNameTexturePath_);
}

void MapTextureEditor::MapLoad() {
	auto values = binaryManager_.Read(saveFileNameMapTexture_);
	if (values.empty()) {
		return;
	}

	saveData_.clear();
	int index = 0;
	while (index < values.size()) {
		int width = BinaryManager::Reverse<int>(values[index++].get());
		int height = BinaryManager::Reverse<int>(values[index++].get());
		std::vector<std::vector<ChipData>> stage{};
		stage.resize(height, std::vector<ChipData>(width));
		for (int i = 0; i < height; ++i) {
			for (int j = 0; j < width; ++j) {
				int textureIndex = BinaryManager::Reverse<int>(values[index++].get());
				Direction direction = static_cast<Direction>(BinaryManager::Reverse<int>(values[index++].get()));
				stage[i][j].textureIndex = textureIndex;
				stage[i][j].direction = direction;
			}
		}
		saveData_.push_back(stage);
	}
}

void MapTextureEditor::MapSave() {
	for (const auto& stage : saveData_) {

		int height = static_cast<int>(stage.size());
		int width = height > 0 ? static_cast<int>(stage[0].size()) : 0;
		binaryManager_.RegisterOutput(width);
		binaryManager_.RegisterOutput(height);

		for (const auto& row : stage) {
			for (const auto& chip : row) {
				binaryManager_.RegisterOutput(chip.textureIndex);
				binaryManager_.RegisterOutput(static_cast<int>(chip.direction));
			}
		}
	}
	binaryManager_.Write(saveFileNameMapTexture_);
}
