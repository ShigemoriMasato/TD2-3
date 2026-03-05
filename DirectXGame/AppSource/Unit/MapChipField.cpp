#include"MapChipField.h"
#include <queue>
#include <cmath>

void MapChipField::SetMapChipData(std::vector<std::vector<TileType>> data) {
	// データを取得
	data_ = data;

	// ブロックの縦と横幅を取得
	kNumBlockVirtical = static_cast<int32_t>(data_.size());
	kNumBlockHorizontal = static_cast<int32_t>(data_[0].size());

	// ユニットの出現位置を求める
	SetHomePosList();
}

void MapChipField::SetDebugMapData() {

	//kNumBlockVirtical = 40;
	//kNumBlockHorizontal = 60;
	//
	//data_.resize(kNumBlockVirtical);
	//for (auto& row : data_) {
	//	row.resize(kNumBlockHorizontal);
	//}
	//
	//// デバックのマップデータを取得
	//for (int32_t z = 0; z < kNumBlockVirtical; ++z) {
	//	for (int32_t x = 0; x < kNumBlockHorizontal; ++x) {
	//		
	//		// 外周をブロックで囲む
	//		if (z == 0 || z == kNumBlockVirtical-1 || x == 0 || x == kNumBlockHorizontal-1) {
	//			data_[z][x] = TileType::Wall;
	//		} else {
	//			data_[z][x] = TileType::Air;
	//		}
	//	}
	//}

	kNumBlockVirtical = 12;
	kNumBlockHorizontal = 24;

	data_.resize(kNumBlockVirtical);
	for (auto& row : data_) {
		row.resize(kNumBlockHorizontal);
	}

	data_ = {
    {TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall},
    {TileType::Wall, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Wall},
    {TileType::Wall, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Wall, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Wall},
    {TileType::Wall, TileType::Air, TileType::Air, TileType::Air, TileType::Road, TileType::Road, TileType::Road, TileType::Road, TileType::Air, TileType::Wall, TileType::Road, TileType::Road, TileType::Road, TileType::Road, TileType::Road, TileType::Road, TileType::Road, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Wall},
    {TileType::Wall, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Wall, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Road, TileType::Road, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Wall},
    {TileType::Wall, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Road, TileType::Road, TileType::Home, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Wall},
    {TileType::Wall, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Road, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Road, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Wall},
    {TileType::Wall, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Road, TileType::Wall, TileType::Road, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Road, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Wall},
    {TileType::Wall, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Road, TileType::Wall, TileType::Road, TileType::Air, TileType::Air, TileType::Road, TileType::Air, TileType::Road, TileType::Road, TileType::Air, TileType::Road, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Wall},
    {TileType::Wall, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Road, TileType::Road, TileType::Road, TileType::Road, TileType::Wall, TileType::Road, TileType::Road, TileType::Road, TileType::Road, TileType::Road, TileType::Road, TileType::Air, TileType::Road, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Wall},
    {TileType::Wall, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Road, TileType::Air, TileType::Air, TileType::Road, TileType::Air, TileType::Air, TileType::Road, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Air, TileType::Wall},
    {TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall, TileType::Wall}
	};

	// ユニットの出現位置を求める
	SetHomePosList();

	//data_[5][4] = TileType::Wall;
	//data_[6][4] = TileType::Wall;
	//data_[5][5] = TileType::Wall;
	//data_[6][5] = TileType::Wall;
}

std::vector<Vector3> MapChipField::GetDirtPosList() {
	std::vector<Vector3> dirtPosList;

	for (int32_t z = 0; z < kNumBlockVirtical; ++z) {
		for (int32_t x = 0; x < kNumBlockHorizontal; ++x) {
			if (data_[z][x] == TileType::Slow) {
				dirtPosList.push_back(GetMapChipPositionByIndex(x, z));
			}
		}
	}
	return dirtPosList;
}

TileType MapChipField::GetBlockTypeByIndex(int32_t xIndex, int32_t zIndex) const {

	// 範囲外を指定されたら空白を返す
	if (xIndex < 0 || kNumBlockHorizontal - 1 < xIndex) {
		return TileType::Air;
	}
	if (zIndex < 0 || kNumBlockVirtical - 1 < zIndex) {
		return TileType::Air;
	}

	return data_[zIndex][xIndex];
}

bool MapChipField::CheckPosInMap(int32_t xIndex, int32_t zIndex) const {
	// 範囲外を指定されたら空白を返す
	if (xIndex < 0 || kNumBlockHorizontal - 1 < xIndex) {
		return false;
	}
	if (zIndex < 0 || kNumBlockVirtical - 1 < zIndex) {
		return false;
	}

	return true;
}

Vector3 MapChipField::GetMapChipPositionByIndex(int32_t xIndex, int32_t zIndex) const {
	return Vector3(kBlockWidth * static_cast<float>(xIndex), 0, static_cast<float>(kBlockHeight * zIndex));
}

void MapChipField::SetMapChipBlockType(int32_t xIndex, int32_t zIndex, TileType type) {
	// データ番号変更
	data_[zIndex][xIndex] = type;
}

MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position) {
	// 指定座標がマップチップで何番目かを返す
	MapChipField::IndexSet indexSet = {};
	indexSet.xIndex = static_cast<int32_t>((position.x + kBlockWidth / 2.0f) / kBlockWidth);
	indexSet.zIndex = static_cast<int32_t>((position.z + kBlockHeight / 2.0f) / kBlockHeight);
	return indexSet;
}

MapChipField::Rect MapChipField::GetRectByIndex(int32_t xIndex, int32_t zIndex) {

	// 指定ブロックの中心座標を取得する
	Vector3 center = GetMapChipPositionByIndex(xIndex, zIndex);

	Rect rect;
	rect.left = center.x - kBlockWidth / 2.0f;
	rect.right = center.x + kBlockWidth / 2.0f;
	rect.bottom = center.y - kBlockHeight / 2.0f;
	rect.top = center.y + kBlockHeight / 2.0f;
	return rect;
}

bool MapChipField::IsBlockHit(MoveDir dir, const CollisionMapInfo& info) {

	// 自キャラの移動後の4つ角の座標を求める
	std::array<Vector3, static_cast<size_t>(Corner::MaxCount)> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(info.pos + info.move, static_cast<Corner>(i),info);
	}

	// 角
	uint32_t corner1 = 0;
	uint32_t corner2 = 0;

	// 移動先
	int32_t moveX = 0;
	int32_t moveZ = 0;

	// 方向に応じて角を求める
	switch (dir)
	{
	case MapChipField::MoveDir::Above:
		corner1 = static_cast<uint32_t>(Corner::kLeftTop);
		corner2 = static_cast<uint32_t>(Corner::kRightTop);
		moveZ = -1;
		break;

	case MapChipField::MoveDir::Below:
		corner1 = static_cast<uint32_t>(Corner::kLeftBottom);
		corner2 = static_cast<uint32_t>(Corner::kRightBottom);
		moveZ = 1;
		break;

	case MapChipField::MoveDir::Right:
		corner1 = static_cast<uint32_t>(Corner::kRightTop);
		corner2 = static_cast<uint32_t>(Corner::kRightBottom);
		moveX = -1;
		break;

	case MapChipField::MoveDir::Left:
		corner1 = static_cast<uint32_t>(Corner::kLeftTop);
		corner2 = static_cast<uint32_t>(Corner::kLeftBottom);
		moveX = 1;
		break;
	}

	TileType blockType;
	TileType blockTypeNext;
	// 角1、角2の当たり判定を行う
	bool hit = false;
	// 角1の点の判定
	MapChipField::IndexSet indexSet;
	indexSet = GetMapChipIndexSetByPosition(positionsNew[corner1]);
	blockType = GetBlockTypeByIndex(indexSet.xIndex, indexSet.zIndex);
	blockTypeNext = GetBlockTypeByIndex(indexSet.xIndex + moveX, indexSet.zIndex + moveZ);
	if ((blockType != TileType::Air && blockType != TileType::Road && blockType != TileType::Slow) &&
		(blockTypeNext == TileType::Air || blockTypeNext == TileType::Road || blockTypeNext == TileType::Slow)) {
		hit = true;
	}
	// 角2の点の判定
	indexSet = GetMapChipIndexSetByPosition(positionsNew[corner2]);
	blockType = GetBlockTypeByIndex(indexSet.xIndex, indexSet.zIndex);
	blockTypeNext = GetBlockTypeByIndex(indexSet.xIndex + moveX, indexSet.zIndex + moveZ);
	if ((blockType != TileType::Air && blockType != TileType::Road && blockType != TileType::Slow) &&
		(blockTypeNext == TileType::Air || blockTypeNext == TileType::Road || blockTypeNext == TileType::Slow)) {
		hit = true;
	}

	return hit;
}

std::vector<Vector3> MapChipField::CalculatePath(const Vector3& start, const Vector3& end) {
	std::vector<Vector3> path;

	IndexSet startIndex = GetMapChipIndexSetByPosition(start);
	IndexSet endIndex = GetMapChipIndexSetByPosition(end);

	// 範囲外チェック
	if (startIndex.xIndex < 0 || startIndex.xIndex >= kNumBlockHorizontal ||
		startIndex.zIndex < 0 || startIndex.zIndex >= kNumBlockVirtical ||
		endIndex.xIndex < 0 || endIndex.xIndex >= kNumBlockHorizontal ||
		endIndex.zIndex < 0 || endIndex.zIndex >= kNumBlockVirtical) {
		return path; // 空のパスを返す
	}

	// ゴールが壁なら、近くの空いている場所を探すか、計算を諦める
	// ここでは単純にゴールが壁なら空のパスを返すことにします
	if (GetBlockTypeByIndex(endIndex.xIndex, endIndex.zIndex) == TileType::Wall) {
		return path;
	}

	// 探索用データの準備
	// 各マスのノード情報を管理する2次元配列 (探索済みかどうかやコストを保持)
	std::vector<std::vector<Node>> nodes(kNumBlockVirtical, std::vector<Node>(kNumBlockHorizontal));

	// 初期化
	for (int z = 0; z < kNumBlockVirtical; ++z) {
		for (int x = 0; x < kNumBlockHorizontal; ++x) {
			nodes[z][x] = { x, z, FLT_MAX, 0.0f, nullptr };
		}
	}

	// スタート地点の設定
	Node* startNode = &nodes[startIndex.zIndex][startIndex.xIndex];
	startNode->gCost = 0.0f;
	startNode->hCost = static_cast<float>(std::abs(endIndex.xIndex - startIndex.xIndex) + std::abs(endIndex.zIndex - startIndex.zIndex)); // マンハッタン距離

	// オープンリスト
	// ポインタを格納し、比較関数を定義
	auto comp = [](Node* a, Node* b) { return a->fCost() > b->fCost(); };
	std::priority_queue<Node*, std::vector<Node*>, decltype(comp)> openList(comp);

	openList.push(startNode);

	// 探索ループ
	// 上下左右の移動方向
	const int dx[] = { 0, 0, -1, 1 };
	const int dz[] = { -1, 1, 0, 0 };

	Node* currentNode = nullptr;

	while (!openList.empty()) {
		// 最もコストが低いノードを取り出す
		currentNode = openList.top();
		openList.pop();

		// ゴールに到達したらループ終了
		if (currentNode->x == endIndex.xIndex && currentNode->z == endIndex.zIndex) {
			break;
		}

		// 周囲4方向をチェック
		for (int i = 0; i < 4; ++i) {
			int nx = currentNode->x + dx[i];
			int nz = currentNode->z + dz[i];

			// マップ範囲外チェック
			if (nx < 0 || nx >= kNumBlockHorizontal || nz < 0 || nz >= kNumBlockVirtical) {
				continue;
			}

			// タイルの種類を取得
			TileType nextTileType = GetBlockTypeByIndex(nx, nz);

			// 壁チェック (Wallなら絶対に通れない)
			if (nextTileType == TileType::Wall) {
				continue;
			}

			float moveCost = 0.0f;
			if (nextTileType == TileType::Road || nextTileType == TileType::Slow) {
				// 道の移動コスト
				moveCost = 1.0f;
			} else {
				// 何もない場所の移動コスト
				moveCost = 5.0f;
			}

			// 新しいGコスト（今のコスト + 計算した移動コスト）
			float newGCost = currentNode->gCost + moveCost;

			// 既により良い経路が見つかっている場合はスキップ
			if (newGCost < nodes[nz][nx].gCost) {
				Node* neighbor = &nodes[nz][nx];
				neighbor->gCost = newGCost;
				neighbor->hCost = static_cast<float>(std::abs(endIndex.xIndex - nx) + std::abs(endIndex.zIndex - nz));
				neighbor->parent = currentNode;

				openList.push(neighbor);
			}
		}
	}

	// 経路が見つかった場合、ゴールから親を辿ってリストを作成
	if (currentNode && currentNode->x == endIndex.xIndex && currentNode->z == endIndex.zIndex) {
		Node* node = currentNode;
		while (node != nullptr) {
			// インデックスをワールド座標に変換して追加
			path.push_back(GetMapChipPositionByIndex(node->x, node->z));
			node = node->parent;
		}
		// ゴール -> スタートの順になっているので反転させる
		std::reverse(path.begin(), path.end());
	}

	return path;
}

std::vector<Vector3> MapChipField::GradeCalculatePath(const Vector3& start, const Vector3& end) {
	std::vector<Vector3> path;

	IndexSet startIndex = GetMapChipIndexSetByPosition(start);
	IndexSet endIndex = GetMapChipIndexSetByPosition(end);

	// 範囲外チェック
	if (startIndex.xIndex < 0 || startIndex.xIndex >= kNumBlockHorizontal ||
		startIndex.zIndex < 0 || startIndex.zIndex >= kNumBlockVirtical ||
		endIndex.xIndex < 0 || endIndex.xIndex >= kNumBlockHorizontal ||
		endIndex.zIndex < 0 || endIndex.zIndex >= kNumBlockVirtical) {
		return path; // 空のパスを返す
	}

	// ゴールが壁なら、近くの空いている場所を探すか、計算を諦める
	// ここでは単純にゴールが壁なら空のパスを返すことにします
	if (GetBlockTypeByIndex(endIndex.xIndex, endIndex.zIndex) == TileType::Wall) {
		return path;
	}

	// 探索用データの準備
	// 各マスのノード情報を管理する2次元配列 (探索済みかどうかやコストを保持)
	std::vector<std::vector<Node>> nodes(kNumBlockVirtical, std::vector<Node>(kNumBlockHorizontal));

	// 初期化
	for (int z = 0; z < kNumBlockVirtical; ++z) {
		for (int x = 0; x < kNumBlockHorizontal; ++x) {
			nodes[z][x] = { x, z, FLT_MAX, 0.0f, nullptr };
		}
	}

	// スタート地点の設定
	Node* startNode = &nodes[startIndex.zIndex][startIndex.xIndex];
	startNode->gCost = 0.0f;
	startNode->hCost = static_cast<float>(std::abs(endIndex.xIndex - startIndex.xIndex) + std::abs(endIndex.zIndex - startIndex.zIndex)); // マンハッタン距離

	// オープンリスト
	// ポインタを格納し、比較関数を定義
	auto comp = [](Node* a, Node* b) { return a->fCost() > b->fCost(); };
	std::priority_queue<Node*, std::vector<Node*>, decltype(comp)> openList(comp);

	openList.push(startNode);

	// 探索ループ
	// 上下左右の移動方向
	const int dx[] = { 0, 0, -1, 1, -1, 1, -1, 1 };
	const int dz[] = { -1, 1, 0, 0, -1, -1, 1, 1 };

	Node* currentNode = nullptr;

	while (!openList.empty()) {
		// 最もコストが低いノードを取り出す
		currentNode = openList.top();
		openList.pop();

		// ゴールに到達したらループ終了
		if (currentNode->x == endIndex.xIndex && currentNode->z == endIndex.zIndex) {
			break;
		}

		// 周囲8方向をチェック
		for (int i = 0; i < 8; ++i) {
			int nx = currentNode->x + dx[i];
			int nz = currentNode->z + dz[i];

			// マップ範囲外チェック
			if (nx < 0 || nx >= kNumBlockHorizontal || nz < 0 || nz >= kNumBlockVirtical) {
				continue;
			}

			// タイルの種類を取得
			TileType nextTileType = GetBlockTypeByIndex(nx, nz);

			// 壁チェック (Wallなら絶対に通れない)
			if (nextTileType == TileType::Wall) {
				continue;
			}

			if (i >= 4) {
				// 斜め移動の場合、縦・横の隣接マスが壁なら斜め移動を許可しない
				TileType tileX = GetBlockTypeByIndex(nx, currentNode->z);
				TileType tileZ = GetBlockTypeByIndex(currentNode->x, nz);

				if (tileX == TileType::Wall || tileZ == TileType::Wall) {
					continue;
				}
			}

			float moveCost = 5.0f;
			if (i >= 4) {
				moveCost *= 1.414f;
			}

			// 新しいGコスト（今のコスト + 計算した移動コスト）
			float newGCost = currentNode->gCost + moveCost;

			// 既により良い経路が見つかっている場合はスキップ
			if (newGCost < nodes[nz][nx].gCost) {
				Node* neighbor = &nodes[nz][nx];
				neighbor->gCost = newGCost;
				neighbor->hCost = static_cast<float>(std::abs(endIndex.xIndex - nx) + std::abs(endIndex.zIndex - nz));
				neighbor->parent = currentNode;

				openList.push(neighbor);
			}
		}
	}

	// 経路が見つかった場合、ゴールから親を辿ってリストを作成
	if (currentNode && currentNode->x == endIndex.xIndex && currentNode->z == endIndex.zIndex) {
		Node* node = currentNode;
		while (node != nullptr) {
			// インデックスをワールド座標に変換して追加
			path.push_back(GetMapChipPositionByIndex(node->x, node->z));
			node = node->parent;
		}
		// ゴール -> スタートの順になっているので反転させる
		std::reverse(path.begin(), path.end());
	}

	return path;
}

Vector3 MapChipField::CornerPosition(const Vector3& center, Corner corner, const CollisionMapInfo& info) {

	// 各角の座標
	Vector3 offsetTable[static_cast<size_t>(Corner::MaxCount)] = {
		Vector3(info.size.x / 2.0f,0.0f, -info.size.z / 2.0f),
		Vector3(-info.size.x / 2.0f,0.0f, -info.size.z / 2.0f),
		Vector3(info.size.x / 2.0f,0.0f, info.size.z / 2.0f),
		Vector3(-info.size.x / 2.0f,0.0f, info.size.z / 2.0f)
	};

	return center + offsetTable[static_cast<int32_t>(corner)];
}

void MapChipField::SetHomePosList() {

	for (size_t z = 0; z < data_.size(); ++z) {
		for (size_t x = 0; x < data_[0].size(); ++x) {
			if (data_[z][x] == TileType::Home) {
				Vector3 pos = { x * kBlockWidth,0.0f,z * kBlockHeight };
				homePosList_.push_back(pos);
			}
		}
	}
}