#include"UnitManager.h"
#include"FpsCount.h"
#include <Common/DebugParam/GameParamEditor.h>

#include"Item/Object/OreItem.h"
#include"Assets/Audio/AudioManager.h"

#ifdef USE_IMGUI
#include <imgui/imgui.h>
#endif

void UnitManager::Initalize(MapChipField* mapChipField, DrawData playerDrawData, int pIndex, DrawData oreDrawData, int oIndex, KeyManager* keyManager, Vector3 playerInitPos, int32_t maxOreNum,
	DrawData spriteDrawData,int unitTex,int playerTex, int32_t mapNum) {

	// ステージがエンドレスの場合、
	if (mapNum >= 0) {
		dagageParam_ = mapNum / 2;

		// 上限設定
		if (dagageParam_ >= 4) {
			dagageParam_ = 4;
		}
	} else {
		dagageParam_ = -1;
	}

	// マップデータを取得
	mapChipField_ = mapChipField;

	// ユニットの出現位置を取得する
	homePosList_ = mapChipField_->GetHomePosList();

	// プレイヤーユニットを初期化
	playerUnit_ = std::make_unique<PlayerUnit>();
	playerUnit_->Init(mapChipField_, playerDrawData,pIndex,playerInitPos, keyManager);

	// おれのモデルデータを取得
	oreDrawData_ = oreDrawData;
	oreTexIndex_ = oIndex;

	// おれの最大数を取得
	maxCurrentOreCount_ = maxOreNum;

	// おれのメモリを確保
	oreUnits_.reserve(maxOreCount_);

	oreRetrunSH_ = AudioManager::GetInstance().GetHandleByName("OreRetrun.mp3");

	// ミニマップのユニットのアイコン描画
	unitIconObjects_.resize(100);
	for (size_t i = 0; i < 100; ++i) {
		unitIconObjects_[i] = std::make_unique<SpriteObject>();
		unitIconObjects_[i]->Initialize(spriteDrawData, { 64.0f,64.0f });
		unitIconObjects_[i]->transform_.position = { -128.0f,0.0f,0.0f };
		unitIconObjects_[i]->color_ = { 1.0f,1.0f,1.0f,1.0f };
		unitIconObjects_[i]->SetTexture(unitTex);
		unitIconObjects_[i]->Update();
	}

	// ミニマップのプレイヤーアイコンの初期化
	playerIconObjects_ = std::make_unique<SpriteObject>();
	playerIconObjects_->Initialize(spriteDrawData, { 96.0f,96.0f });
	playerIconObjects_->transform_.position = { -128.0f,0.0f,0.0f };
	playerIconObjects_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	playerIconObjects_->SetTexture(playerTex);
	playerIconObjects_->Update();

#ifdef USE_IMGUI
	// 値の登録
	RegisterDebugParam();
#endif
	ApplyDebugParam();
	// 値の適応
	//maxCurrentOreCount_ = GameParamEditor::GetInstance()->GetValue<int32_t>(kGroupName_, "MaxOreCount");
}

void UnitManager::Update() {
#ifdef USE_IMGUI
	// 値の適応
	ApplyDebugParam();
#endif

	// ユニットマーククラスのリセット
	unitMarkUIManager_->Reset();

	// ユニットのUI
	oreUnitHPUI_->Update();

	// 鉱石位置をリセット
	unitEffectManager_->Reset();

	// ユニットの出撃処理
	UnitSpawn();

	// プレイヤーの更新処理
	playerUnit_->Update();

	// プレイヤー位置を設定
	unitMarkUIManager_->SetPlayerPos(*playerUnit_->GetPos());

	// おれユニットの更新処理
	for (auto& [id, unit] : oreUnits_) {
		// 生きている場合は更新
		if (unit->IsActive() && !unit->IsDead()) {
			// ユニットの更新処理
			unit->Update();

			// HPを追加
			if (unit->GetState() != OreUnit::State::Return) {
				oreUnitHPUI_->Add(unit->GetPos() + Vector3(0.0f, 2.0f, 0.0f), unit->GetHp(), unit->GetMaxHp());

				// ユニットの位置を設定
				unitPosList_.push_back(unit->GetPos());
			}

			// 回収中は鉱石を持たせる
			if (unit->GetState() == OreUnit::State::ToDeliver) {
				unitEffectManager_->AddOreItem(unit->GetPos() + Vector3(0.0f, 1.0f, 0.0f));
			}

			// 更新して有効フラグがfalseだったら、再利用リストに追加する
			if (!unit->IsActive()) {
				// 再利用リストに追加
				freeIndices_.push_back(id);
				activeCount_--;

				AudioManager::GetInstance().Play(oreRetrunSH_, 0.5f, false);

			} else if (unit->IsDead()) {

				// 死亡ログを出す
				logUI_->AddUnitDeathLog();

				// 再利用リストに追加
				freeIndices_.push_back(id);
				activeCount_--;
				// 死亡していた場合、利用出来る最大数を減らす
				maxCurrentOreCount_--;
			}
		}
	}

	// ユニットの演出を更新
	unitEffectManager_->Update();

	// ユニットのマーク処理を更新
	unitMarkUIManager_->Update();
}

void UnitManager::MiniMapUpdate() {
	// ユニットマーククラスのリセット
	unitMarkUIManager_->Reset();

	// ミニマップ用の座標をクリア
	unitPosList_.clear();
	unitIconIndex_ = -1;

	// おれユニットの更新処理
	for (auto& [id, unit] : oreUnits_) {
		// 生きている場合は更新
		if (unit->IsActive() && !unit->IsDead()) {
		
			// ユニットの更新処理
			unit->Update();
			// HPを追加
			if (unit->GetState() != OreUnit::State::Return) {

				// ユニットの位置を設定
				unitPosList_.push_back(unit->GetPos());
			}
		}
	}

	// ユニットのアイコンを作成
	ProcessClusters();

	// プレイヤーアイコンの作成
	MarkerResult marker = miniMap_->GetMarkerInfo(*playerUnit_->GetPos(), 36.0f);
	playerIconObjects_->transform_.position = Vector3(marker.position.x, marker.position.y, 0.0f);
	playerIconObjects_->Update();

	// ユニットのマーク処理を更新
	unitMarkUIManager_->Update(false);
}

void UnitManager::Draw(Window* window, const Matrix4x4& vpMatrix) {

	// プレイヤーを描画
	playerUnit_->Draw(window, vpMatrix);

	// おれを描画
	for (const auto& [id, unit] : oreUnits_) {
		if (!unit->IsActive()) { continue; }
		unit->Draw(window, vpMatrix);
	}
}

void UnitManager::DrawUI() {

#ifdef USE_IMGUI

	ImGui::Begin("OreUnitsInfo");
	ImGui::Text("MaxNumInstace : %d", maxOreCount_);
	ImGui::Text("MaxCurrentNum : %d", maxCurrentOreCount_);
	ImGui::Text("OreUnitInstace : %d", static_cast<int>(oreUnits_.size()));
	ImGui::Text("ActiveNum : %d", activeCount_);
	ImGui::End();
#endif
}

void UnitManager::DrawIcon(Window* window, const Matrix4x4& vpMatrix) {

	// プレイヤーアイコンの描画
	playerIconObjects_->Draw(window, vpMatrix);

	// ミニマップ用のユニットアイコン
	if (unitIconIndex_ != -1) {
		for (size_t i = 0; i <= unitIconIndex_; ++i) {
			unitIconObjects_[i]->Draw(window, vpMatrix);
		}
	}
}

void UnitManager::MovePlayer(const Vector3& pos) {

	// 指定した位置のブロックの種類を取得する
	MapChipField::IndexSet index =  mapChipField_->GetMapChipIndexSetByPosition(pos);
	if (!mapChipField_->CheckPosInMap(index.xIndex, index.zIndex)) { return; }
	TileType type = mapChipField_->GetBlockTypeByIndex(index.xIndex,index.zIndex);

	// 移動可能な場合
	if (type != TileType::Wall && type != TileType::Home &&
		type != TileType::Gold && type != TileType::MediumGold && type != TileType::LargeGold) {

		// プレイヤーの自動移動を設定
		playerUnit_->SetMovePos(pos);
	}
}

void UnitManager::PressMovePlayer(const Vector3& pos) {

	playerUnit_->SetPressMove(pos);
}

void UnitManager::RegisterUnit(const Vector3& targetPos, const int32_t& spawnNum, const int32_t& excessNum, OreItem* oreItem) {

	// 生成出来る最大の数を超えていれば、早期リターン
	if (activeCount_ >= maxCurrentOreCount_) {
		return;
	}

	SpawnData data;
	data.currentNum = 0;
	data.spawnNum = spawnNum + excessNum;
	data.pos = targetPos;
	data.oreItem_ = oreItem;
	data.groupId_ = nextGroupIndex_++;

	// ユニットのアクティブカウントを加算
	activeCount_ += data.spawnNum;

	// 登録
	spawnList_.push_back(data);
}

void UnitManager::AddOreUnit(const Vector3& targetPos, OreItem* oreItem,uint32_t groupId) {

	// 出現位置を求める
	Vector3 homePos = GetNearHomePos(targetPos);

	// 拠点をアニメーションさせる
	HomeManager_->SetAnimation(homePos);

	bool isReuse = false;
	int32_t index;
	if (!freeIndices_.empty()) {
		index = freeIndices_.front();
		freeIndices_.pop_front();
		isReuse = true;
	} else {
		if (currentId_ >= maxOreCount_ || oreUnits_.size() >= maxCurrentOreCount_) {
			return;
		}
		index = currentId_++;
	}

	if (isReuse) {
		// 既存のユニットを再利用
		auto unit = oreUnits_.find(index);
		if (unit == oreUnits_.end()) {
			assert(false && "Not found Unit");
		}
		// 初期化
		unit->second->Init(homePos, targetPos, oreItem, groupId);

	} else {
		// 新しく登録
		std::unique_ptr<OreUnit> oreUnit = std::make_unique<OreUnit>(mapChipField_, oreDrawData_, oreTexIndex_, playerUnit_->GetPos(), unitMarkUIManager_, unitEffectManager_, dagageParam_);
		oreUnit->Init(homePos, targetPos, oreItem, groupId);

		oreUnits_[index] = std::move(oreUnit);
	}
}

void UnitManager::UnitSpawn() {

	// スポーン状態でなければ、設定
	if (!isSpawn_) {

		if (spawnList_.empty()) { return; }

		isSpawn_ = true;
		spawnData_ = spawnList_.front();
		spawnList_.pop_front();
		spawnTimer_ = 1.0f;
	}

	// 一定間隔で出撃させる
	if (spawnData_.currentNum < spawnData_.spawnNum) {

		spawnTimer_ += FpsCount::deltaTime / spawnTime_;

		if (spawnTimer_ >= 1.0f) {
			AddOreUnit(spawnData_.pos,spawnData_.oreItem_, spawnData_.groupId_);
			spawnData_.currentNum++;
			spawnTimer_ = 0.0f;
		}
	} else {
		isSpawn_ = false;
	}
}

Vector3 UnitManager::GetNearHomePos(const Vector3& targetPos) {

	if (homePosList_.empty()) {
		assert(0 && "Not HomePosList");
	}

	float minDis = FLT_MAX;
	Vector3 nearPos = homePosList_[0];

	for (const auto& pos : homePosList_) {
		float d = DistanceXZ(pos, targetPos);

		if (d < minDis) {
			minDis = d;
			nearPos = pos;
		}
	}

	return nearPos;
}

void UnitManager::ProcessClusters() {
	// 飛ばす
	if (unitPosList_.empty()) { return; }

	std::vector<Cluster> clusters;

	float thresholdSq = 3.0f * 3.0f;

	for (const auto& pos : unitPosList_) {
		bool merged = false;

		// 既存のクラスタに近いかチェック
		for (auto& cluster : clusters) {
			// 現在のクラスタの平均位置を計算
			Vector3 currentCenter = {
				cluster.positionSum.x / cluster.count,
				cluster.positionSum.y / cluster.count,
				cluster.positionSum.z / cluster.count
			};

			// 距離チェック
			Vector3 diff = {
				pos.x - currentCenter.x,
				pos.y - currentCenter.y,
				pos.z - currentCenter.z
			};

			if (LengthSq(diff) <= thresholdSq) {
				// 近いのでグループに加算
				cluster.positionSum.x += pos.x;
				cluster.positionSum.y += pos.y;
				cluster.positionSum.z += pos.z;
				cluster.count++;
				merged = true;
				break;
			}
		}

		// どのグループにも属さない場合は新規作成
		if (!merged) {
			Cluster newCluster;
			newCluster.positionSum = pos;
			newCluster.count = 1;
			clusters.push_back(newCluster);
		}
	}

	// UIの生成
	for (const auto& cluster : clusters) {
		Vector3 finalPos = {
			cluster.positionSum.x / cluster.count,
			cluster.positionSum.y / cluster.count,
			cluster.positionSum.z / cluster.count
		};

		// アイコンを追加
		unitIconIndex_++;
		MarkerResult marker = miniMap_->GetMarkerInfo(finalPos, 36.0f);
		unitIconObjects_[unitIconIndex_]->transform_.position = Vector3(marker.position.x, marker.position.y,0.0f);
		unitIconObjects_[unitIconIndex_]->Update();
	}
}

void UnitManager::RegisterDebugParam() {
	// 登録
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "SpawnNum", unitSpawnNum_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "SpawnTime", spawnTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "MaxOreCount", maxCurrentOreCount_);
}

void UnitManager::ApplyDebugParam() {
	// 適応
	unitSpawnNum_ = GameParamEditor::GetInstance()->GetValue<uint32_t>(kGroupName_, "SpawnNum");
	spawnTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "SpawnTime");
}