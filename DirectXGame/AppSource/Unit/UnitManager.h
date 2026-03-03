#pragma once
#include<unordered_map>
#include <deque>
#include"Common/KeyConfig/KeyManager.h"

#include"Unit/Player/PlayerUnit.h"
#include"Unit/Ore/OreUnit.h"

#include"UI/LogUI.h"
#include"UI/OreUnitHPUI.h"
#include"UnitEffectManager.h"

#include"Item/HomeManager.h"

#include"UnitMarkUIManager.h"

#include"Game/MiniMap/MiniMap.h"

class OreItem;

class UnitManager {
public:

	// 出現させるためのデータ
	struct SpawnData {
		uint32_t spawnNum = 1;  // 出現させる数
		uint32_t currentNum = 0; // 現在の出現数
		Vector3 pos = {}; // 出現位置
		OreItem* oreItem_ = nullptr;
		uint32_t groupId_ = 0; // グループId
	};

	struct Cluster {
		Vector3 positionSum; // 位置の合計
		int count;           // まとめた数
	};

public:

	void Initalize(MapChipField* mapChipField, DrawData playerDrawData,int pIndex, DrawData oreDrawData,int oIndex, KeyManager* keyManager, Vector3 playerInitPos,int32_t maxOreNum,
		DrawData spriteDrawData, int unitTex, int playerTex,int32_t mapNum);

	void Update();

	void MiniMapUpdate();

	void Draw(Window* window, const Matrix4x4& vpMatrix);

	void DrawUI();

	// ミニマップ用にアイコンを描画
	void DrawIcon(Window* window, const Matrix4x4& vpMatrix);

	/// <summary>
	/// プレイヤーを移動させる
	/// </summary>
	/// <param name="pos"></param>
	void MovePlayer(const Vector3& pos);

	/// <summary>
	/// ユニットを出動させる
	/// </summary>
	/// <param name="targetPos">移動する目標位置</param>
	/// <param name="excessNum">余剰の数</param>
	void RegisterUnit(const Vector3& targetPos,const int32_t& spawnNum,const int32_t& excessNum, OreItem* oreItem);

	// 出撃させるユニットの数
	int32_t GetUnitSpawnNum() const {
		// 現在の空き枠を計算する
		int32_t remainingSpace = maxCurrentOreCount_ - activeCount_;
		if (remainingSpace <= 0) {
			return 0;
		}

		if (remainingSpace < (int32_t)unitSpawnNum_) {
			return remainingSpace;
		}

		return unitSpawnNum_;
	}

	// 最大の数
	int32_t GetMaxOreCount() const { return maxCurrentOreCount_; }

	// 現在出撃している数
	int32_t GetOreCount() const { return activeCount_; }

	// UIシステムを取得
	void SetUnitHp(OreUnitHPUI* oreUnitHPUI) {
		oreUnitHPUI_ = oreUnitHPUI;
	}

	// 演出システムを取得
	void SetUnitEffect(UnitEffectManager* unitEffectManager) {
		unitEffectManager_ = unitEffectManager;
	}

	// 拠点管理クラスを取得
	void SetHomeManager(HomeManager* home) {
		HomeManager_ = home;
	}

	// UIマーク
	void SetUnitMarkUI(UnitMarkUIManager* unitMarkUIManager){
		unitMarkUIManager_ = unitMarkUIManager;
	}

	Vector3 GetPlayerPosition() { return *playerUnit_->GetPos(); }

	// ログシステムを取得
	void SetLogUI(LogUI* logUI) {
		logUI_ = logUI;

		unitEffectManager_->SetCollBack([this]() {
			logUI_->AddUnitConflictLog();
		});
	}

	// ミニマップを作成
	void SetMinMap(MiniMap* miniMap) {
		miniMap_ = miniMap;
	}

private:
	// マップ
	MapChipField* mapChipField_ = nullptr;
	// ユニットのHp表示
	OreUnitHPUI* oreUnitHPUI_ = nullptr;
	// ユニットの演出表示
	UnitEffectManager* unitEffectManager_ = nullptr;
	// 拠点管理クラス
	HomeManager* HomeManager_ = nullptr;
	// ユニットマークUI
	UnitMarkUIManager* unitMarkUIManager_ = nullptr;
	// ログを出力するクラス
	LogUI* logUI_ = nullptr;

	// ミニマップ
	MiniMap* miniMap_ = nullptr;

private:

	// おれの描画データ
	DrawData oreDrawData_;
	int oreTexIndex_ = 0;

	// プレイヤー
	std::unique_ptr<PlayerUnit> playerUnit_;

	// おれ
	// <id, インスタンス>
	std::unordered_map<int32_t, std::unique_ptr<OreUnit>> oreUnits_;
	// 最大のおれの数
	int32_t maxOreCount_ = 100;
	// 現在出せる最大のおれの数
	int32_t maxCurrentOreCount_ = 20;
	// 現在のid
	int32_t currentId_ = 0;
	// 再利用可能id
	std::deque<int32_t> freeIndices_;

	// おれの出現位置
	std::vector<Vector3> homePosList_;

	int32_t activeCount_ = 0;

	// 出現するユニットをを登録
	std::deque<SpawnData> spawnList_;
	bool isSpawn_ = false;
	SpawnData spawnData_;
	float spawnTimer_ = 0.0f;

	// グループIdを取得
	uint32_t nextGroupIndex_ = 0;

	// デバック用
	std::string kGroupName_ = "UnitManager";

	// ユニットアイコン
	std::vector<std::unique_ptr<SpriteObject>> unitIconObjects_;
	std::vector<Vector3> unitPosList_;
	int32_t unitIconIndex_ = -1;

	// プレイヤーアイコン
	std::unique_ptr<SpriteObject> playerIconObjects_;

	int32_t dagageParam_ = 0;

	/// 変える音

	// 帰る音
	uint32_t oreRetrunSH_ = 0;

private: // 調整項目

	// 出現させるユニットの数
	uint32_t unitSpawnNum_ = 1;

	// 出現させる時間間隔
	float spawnTime_ = 0.2f;

private:

	/// <summary>
	/// 登録されたユニットを出現させる
	/// </summary>
	void UnitSpawn();

	/// <summary>
	/// ユニットを追加
	/// </summary>
	/// <param name="targetPos">移動する位置</param>
	void AddOreUnit(const Vector3& targetPos, OreItem* oreItem, uint32_t groupId);

	// 一番近い出現位置を求める
	Vector3 GetNearHomePos(const Vector3& targetPos);

	// 位置をまとめる
	void ProcessClusters();

private:

	/// <summary>
	/// 値を登録する
	/// </summary>
	void RegisterDebugParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam();
};

// ヘルプ関数
namespace {

	// XZ上での距離を求める
	float DistanceXZ(const Vector3& v1, const Vector3& v2) {
		float dx = v1.x - v2.x;
		float dz = v1.z - v2.z;
		return dx * dx + dz * dz;
	}
}