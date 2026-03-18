#pragma once
#include <Scene/IScene.h>
#include <Camera/DebugCamera.h>
#include <Common/DebugParam/ParamManager.h>
#include <Render/PostEffect.h>
#include <Collision/ColliderManager.h>

#include "Unit/UnitManager.h"
#include "Unit/MapChipField.h"
#include "Unit/MapChipRender.h"
#include "Item/OreItemManager.h"
#include "UI/FadeTransition.h"
#include "UI/Object/FontObject.h"
#include "UI/GameUIManager.h"
#include "TimeTracker.h"
#include "UI/OreUnitHPUI.h"
#include "Unit/UnitEffectManager.h"
#include "Item/HomeManager.h"
#include "UI/StartCountUI.h"
#include "Unit/UnitMarkUIManager.h"
#include"UI/LogUI.h"

#include <Game/MiniMap/MiniMap.h>
#include <ModelEditScene/Render/MapRender.h>
#include <ModelEditScene/Render/DebugMCRender.h>
#include <ModelEditScene/Render/StaticObjectRender.h>
#include <GameOver/GameOverTask.h>

#include "GameCamera/CameraController.h"

// ゲームオーバーシーン
#include "UI/GameOverUI.h"
// クリアシーン
#include "UI/ClearUI.h"
// ポーズシーン
#include "UI/PauseUI.h"

class GameScene : public IScene {
public:
	~GameScene();

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private://エンジン的に必要な奴ら

	DualDisplay* display_ = nullptr;
	GameWindow* gameWindow_ = nullptr;

private:

	std::unique_ptr<DebugCamera> debugCamera_ = nullptr;
	Camera camera_;

	// ui描画用のカメラ
	std::unique_ptr<Camera> uiCamera_;

	// 当たり判定管理クラス
	std::unique_ptr<ColliderManager> colliderManager_;

	int param1;
	float param2;
	Vector3 param3;
	Vector4 param4;
	std::string param5;

	std::unique_ptr<ParamManager> paramManager_ = nullptr;

	std::unique_ptr<PostEffect> postEffect_ = nullptr;
	PostEffectConfig postEffectConfig_{};
	Fade fade_{};

	// シーン切り替え
	bool isSceneChange_ = false;
	bool isRetry_ = true;

	// 遷移の処理
	std::unique_ptr<FadeTransition> fadeTransition_;

	// ゲームのBGMハンドル
	uint32_t gameBGMSH_ = 0;

	float pressTimer_ = 0.0f;

	// 入力をしているか判断するフラグ
	bool isInputPress_ = false;

	//=====================================================
	// インゲーム
	//=====================================================

	// ゲームカメラの管理
	std::unique_ptr<CameraController> cameraController_;

	// マップデータの中間管理職
	std::unique_ptr<MapChipField> mapChipField_;

	// マップの描画
	std::unique_ptr<MapRender> mapRender_;
	std::unique_ptr<DebugMCRender> debugMapRender_;//マップチップを単純な色で描画
	std::map<TileType, Vector3> colorMap_;//DebugMCRenderに使う、マップチップの色
	std::unique_ptr<StaticObjectRender> staticObjectRender_;//マップ上の動かないオブジェクトを描画

	// MiniMap
	std::unique_ptr<MiniMap> miniMap_;
	bool isActiveMinMap_ = false;

	// 現在のマップ
	CurrentStageConfig currentMap_;

	// 鉱石の管理
	std::unique_ptr<OreItemManager> oreItemManager_;

	// ユニットの拠点管理クラス
	std::unique_ptr<HomeManager> homeManager_;

	// ユニット管理
	std::unique_ptr<UnitManager> unitManager_;

	// ユニットの演出管理(仮で作ったため消すかも)
	std::unique_ptr<UnitEffectManager> unitEffectManager_;

	// ゲームで仕様するHI管理クラス
	std::unique_ptr<GameUIManager> gameUIManager_;

	// ユニットのHPUI
	std::unique_ptr<OreUnitHPUI> oreUnitHpUI_;

	// 時間を測る
	std::unique_ptr<TimeTracker> timeTracker_;
	bool isTimerSet_ = false;

	// カウントUI
	std::unique_ptr<StartCountUI> startCountUI_;

	// ユニットに関するマークUI
	std::unique_ptr<UnitMarkUIManager> unitMarkUIManager_;

	// ログUI
	std::unique_ptr<LogUI> logUI_;

	/// 音声処理

	// ユニットを配置出来ない時
	uint32_t oreRejectedSH_ = 0;

	// ユニットの出撃音
	uint32_t oreGoSH_ = 0;

	// タイムアップ音声
	uint32_t timeUpSH_ = 0;
	bool isPlayTimeUpSH_ = false;

	// 操作ガイドUI
	std::unique_ptr<SpriteObject> guideSprite_;
	// 開く
	bool isOpenGuide_ = false;

	//================================================
	// ゲームオーバーシーン
	//================================================

	// ゲームオーバーUI
	std::unique_ptr<GameOverUI> gameOverUI_;

	std::unique_ptr<GameOverTask> gameOverTask_;

	// ゲームオーバーシーン
	bool isGameOver_ = false;

	//================================================
	// クリアシーン
	//================================================

	// クリアUI
	std::unique_ptr<ClearUI> clearUI_;

	// クリアシーンを判断
	bool isGameClear_ = false;

	// クリア音声
	uint32_t clearSH_ = 0;
	bool isPlayClearSH_ = false;

	//================================================
	// ポーズシーン
	//================================================

	// ポーズシーン
	std::unique_ptr<PauseUI> pauseUI_;

	// ポーズシーン
	bool isPauseScene_ = false;

	//================================================
	// Tool系
	//================================================
	//Debug用ColorMapのためのやつ
	BinaryManager binaryManager_;

private: // デバックの調整項目

	// 時間
	float mTime_ = 1.0f;
	float sTime_ = 30.0f;

	// 最大の鉱石数
	int32_t maxOreItemNum_ = 20;

	bool selectSceneBack_ = false;

private:

	// 恒常的な更新処理
	void CommonUpdate();

	// ゲームの実行処理
	void InGameScene();

	// 他のシーンの初期化処理
	void InitializeOtherScene();

	/// <summary>
	/// 値を登録する
	/// </summary>
	void RegisterDebugParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam();

	/// <summary>
	/// ColorMapを読み込む
	/// </summary>
	void LoadDebugColorMap();

	/// <summary>
	/// 鉱床の配置
	/// </summary>
	void PutGold();

	/// <summary>
	/// Positionの初期位置を取得
	/// </summary>
	/// <returns>初期位置</returns>
	Vector3 GetPlayerInitPosition();

	/// <summary>
	/// ランキング等のデータを保存する
	/// </summary>
	void SaveGameData();
};
