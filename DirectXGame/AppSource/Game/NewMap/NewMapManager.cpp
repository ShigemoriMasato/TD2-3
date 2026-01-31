#include "NewMapManager.h"

void NewMapManager::Initialize(ModelManager* modelManager) {
	modelManager_ = modelManager;
	LoadTexturePaths();
	LoadMapData();
	LoadModelList();
	LoadDecoData();
	LoadStageConfig();
}

CurrentStageConfig NewMapManager::GetStageMap(int stage, int map) {
	CurrentStageConfig config;
	if(stageData_.size() <= stage) {
		config.initOreNum = 10;
		config.norma = 10000000;
		config.currentMap = newMapData_[0];
		config.currentMapID = 0;
		return config;
	}
	map = std::clamp(map, 0, int(stageData_[stage].configs.size()));
	int mapID = stageData_[stage].configs[map].mapID;

	config.initOreNum = stageData_[stage].initOreNum;
	config.norma = stageData_[stage].configs[map].norma;
	config.currentMapID = mapID;
	config.currentMap = newMapData_[mapID];

	return config;
}

void NewMapManager::LoadTexturePaths() {
	auto values = binaryManager_.Read(saveFileNameTexturePath_);
	if (values.empty()) {
		return;
	}
	int index = 0;

	while (index < values.size()) {
		int textureID = BinaryManager::Reverse<int>(values[index++].get());
		std::string texturePath = BinaryManager::Reverse<std::string>(values[index++].get());
		textureMap_[textureID] = texturePath;
	}
}

void NewMapManager::LoadMapData() {
	auto mapValues = binaryManager_.Read(saveFileNameMapData_);
	auto textureValues = binaryManager_.Read(saveFileNameTextureMap_);
	if (mapValues.empty() || textureValues.empty()) {
		return;
	}

	int mapIndex = 0;
	int textureIndex = 0;

	while(mapIndex < mapValues.size() && textureIndex < textureValues.size()) {
		NewMap newMap;

		//マップの大きさの読み込み
		int width = BinaryManager::Reverse<int>(mapValues[mapIndex++].get());
		int height = BinaryManager::Reverse<int>(mapValues[mapIndex++].get());

		int texWidth = BinaryManager::Reverse<int>(textureValues[textureIndex++].get());
		int texHeight = BinaryManager::Reverse<int>(textureValues[textureIndex++].get());

		//大きさが合わなかったら読み込み失敗
		if(texWidth != width || texHeight != height) {
			return;
		}

		//MapChipデータ読み込み
		newMap.mapChipData.resize(height, std::vector<TileType>(width));
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				int tileTypeInt = BinaryManager::Reverse<int>(mapValues[mapIndex++].get());
				newMap.mapChipData[y][x] = static_cast<TileType>(tileTypeInt);
			}
		}

		//描画データ読み込み
		newMap.renderData.resize(height, std::vector<ChipData>(width));
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				int textureID = BinaryManager::Reverse<int>(textureValues[textureIndex++].get());
				int directionInt = BinaryManager::Reverse<int>(textureValues[textureIndex++].get());
				newMap.renderData[y][x] = { textureID, static_cast<Direction>(directionInt) };
			}
		}

		newMapData_.push_back(newMap);
	}
}

void NewMapManager::LoadDecoData() {
	auto values = binaryManager_.Read(saveFileNameDecorationData_);

	if (values.empty()) {
		return;
	}

	int index = 0;

	int mapCount = BinaryManager::Reverse<int>(values[index++].get());
	newMapData_.resize(mapCount);

	for (int i = 0; i < mapCount; ++i) { //マップの数だけfor

		int decorationCount = BinaryManager::Reverse<int>(values[index++].get());

		for (int j = 0; j < decorationCount; ++j) {// モデルの数だけfor

			int modelID = BinaryManager::Reverse<int>(values[index++].get());
			int modelIndex = modelMap_[modelID].first;
			int transformCount = BinaryManager::Reverse<int>(values[index++].get());
			std::vector<Transform> transforms;

			for (int k = 0; k < transformCount; ++k) { //配置された数だけfor
				Transform transform;
				transform.position = BinaryManager::Reverse<Vector3>(values[index++].get());
				transform.rotate = BinaryManager::Reverse<Vector3>(values[index++].get());
				transform.scale = BinaryManager::Reverse<Vector3>(values[index++].get());
				transforms.push_back(transform);
			}

			newMapData_[i].decorations[modelIndex] = transforms;
		}

	}
}

void NewMapManager::LoadModelList() {
	auto values = binaryManager_.Read(saveFileNameModelList_);
	if (values.empty()) {
		return;
	}
	int index = 0;
	while (index < values.size()) {
		int modelID = BinaryManager::Reverse<int>(values[index++].get());
		std::string modelPath = BinaryManager::Reverse<std::string>(values[index++].get());
		int modelIndex = modelManager_->LoadModel(".Deco/" + modelPath);
		modelMap_[modelID] = { modelIndex, modelPath };

		//colorは使わないので読み飛ばし
		++index;
	}
}

void NewMapManager::LoadStageConfig() {
	auto values = binaryManager_.Read(saveFileNameStageData_);
	if (values.empty()) {
		return;
	}
	int index = 0;

	while (index < values.size()) {
		StageData& stage = stageData_.emplace_back();
		int initOre = BinaryManager::Reverse<int>(values[index++].get());
		int mapSize = BinaryManager::Reverse<int>(values[index++].get());
		std::vector<MapConfig> configs;
		configs.resize(mapSize);
		for (int i = 0; i < mapSize; ++i) {
			int mapID = BinaryManager::Reverse<int>(values[index++].get());
			int norma = BinaryManager::Reverse<int>(values[index++].get());
			configs[i] = { mapID, norma };
		}
		
		stage.initOreNum = initOre;
		stage.configs = configs;
	}
}
