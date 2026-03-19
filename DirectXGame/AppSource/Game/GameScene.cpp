#include "GameScene.h"
#include <Common/DebugParam/GameParamEditor.h>
#include"FpsCount.h"
#include"GameCamera/DebugMousePos.h"
#include"Item/OreItemStorageNum.h"
#include"LightManager.h"
#include <Utility/Easing.h>
#include<Game/SelectScene.h>
#include <OreAddScene/OreAddScene.h>
#include"Assets/Audio/AudioManager.h"
#include <ResultScene/ResultScene.h>

namespace {
	//MiniMap確認用
	bool hasNextMap_ = true;

	std::string fontName = "YDWbananaslipplus.otf";

	std::string playerModelName = "Player";

	std::string oreModelName = "Ore";

	std::string mineralModelName = "Mineral";
	std::string mineralItemModelName = "MineralItem";

	std::string spriteModelName = "Sprite";

	std::string unitHpModelName = "UnitHp";

	std::string oreBaseModelName = "OreBase";

	std::string clickName = "ClickMark";
}

GameScene::~GameScene() {
	//lightの初期化
	LightManager::GetInstance()->Initialize();

	isSceneChange_ = false;
	isGameClear_ = false;
	isGameOver_ = false;

	// ゲームシーンで取得した鉱石の数
	commonData_->goldNum = OreItemStorageNum::currentOreItemNum_;
	// ゲームシーンで残っているおれ
	commonData_->oreNum = unitManager_->GetMaxOreCount();

	OreItemStorageNum::currentOreItemNum_ = 0;

	// BGMを止める
	if (AudioManager::GetInstance().IsPlay(gameBGMSH_)) {
		AudioManager::GetInstance().Stop(gameBGMSH_);
	}
}

void GameScene::Initialize() {
	gameWindow_ = commonData_->mainWindow.get();
	display_ = commonData_->display.get();
	//カメラの初期化
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
	//パラメーター管理の初期化
	paramManager_ = std::make_unique<ParamManager>();

	//lightの初期化
	LightManager::GetInstance()->Initialize();

	//　当たり判定管理クラスを初期化
	colliderManager_ = std::make_unique<ColliderManager>();
	// 当たり判定管理クラスを登録
	Collider::SetColliderManager(colliderManager_.get());

	// BGMを発生
	gameBGMSH_ = AudioManager::GetInstance().GetHandleByName("GameBGM.mp3");
	AudioManager::GetInstance().Play(gameBGMSH_, 0.5f, true);

	// タイムアップ音声
	timeUpSH_ = AudioManager::GetInstance().GetHandleByName("Finish.mp3");
	// クリア音声
	clearSH_ = AudioManager::GetInstance().GetHandleByName("NormaClear.mp3");
	// ユニットを鉱石に配置出来ない時
	oreRejectedSH_ = AudioManager::GetInstance().GetHandleByName("OreRejected.mp3");
	// ユニットの出撃音
	oreGoSH_ = AudioManager::GetInstance().GetHandleByName("Ore_Instructions.mp3");

	auto inst = GameParamEditor::GetInstance();
	inst->SetActiveScene("GameScene");
	inst->CreateGroup("Test", "GameScene");
	inst->AddItem("Test", "param1", param1, 0);
	inst->AddItem("Test", "param2", param2, 1);
	inst->AddItem("Test", "param3", param3, 2);
	inst->AddItem("Test", "param4", param4, 3);
	inst->AddItem("Test", "param5", param5, 4);

	// UIカメラを設定
	uiCamera_ = std::make_unique<Camera>();
	uiCamera_->SetProjectionMatrix(OrthographicDesc{});
	uiCamera_->MakeMatrix();

	//==================================================
	// カメラシステム
	//==================================================

	// プレイヤーモデルを取得
	int clickModelID = modelManager_->LoadModel(clickName);
	auto clickModel = modelManager_->GetNodeModelData(clickModelID);

	// ゲームカメラを設定
	cameraController_ = std::make_unique<CameraController>();
	cameraController_->Initialize(input_, drawDataManager_->GetDrawData(clickModel.drawDataIndex), 0);

	//============================================
	// マップシステム
	//============================================
	hasNextMap_ = true;
	if (commonData_->isEndlessMode) {
		currentMap_ = commonData_->newMapManager->GetEndlessMap(commonData_->stageCount, commonData_->prevMapIndex);

		if (commonData_->stageCount == 0) {
			commonData_->oreNum = 20;
		}

	} else {
		currentMap_ = commonData_->newMapManager->GetStageMap(commonData_->nextStageIndex, commonData_->nextMapIndex);

		if (commonData_->nextMapIndex == 0) {
			commonData_->oreNum = currentMap_.initOreNum;
		}

		if (currentMap_.currentMapID != commonData_->nextMapIndex) {
			Logger logger = getLogger("GameScene");
			logger->info("MapDataError: Stage {} Map {}", commonData_->nextStageIndex, commonData_->nextMapIndex);
		}

		commonData_->nextMapIndex++;
		auto map = commonData_->newMapManager->GetStageMap(commonData_->nextStageIndex, commonData_->nextMapIndex);
		if (map.currentMapID == -1) {
			hasNextMap_ = false;
		}
	}
	commonData_->prevMapIndex = currentMap_.currentMapID;
	commonData_->stageCount++;

	commonData_->norma = currentMap_.norma;

	//ここでOreの情報を更新しておく
	commonData_->maxOreNum = std::max(commonData_->maxOreNum, currentMap_.initOreNum);

	Vector3 playerInitPos = GetPlayerInitPosition();

	// マップデータ解釈機能を初期化
	mapChipField_ = std::make_unique<MapChipField>();
	// マップデータの受け取り
	mapChipField_->SetMapChipData(currentMap_.currentMap.mapChipData);


	// マップの最大サイズを取得
	cameraController_->SetMapMaxSize(mapChipField_->GetMaxMapSize());


	// Cubeモデルを取得
	auto wallModel = modelManager_->GetNodeModelData(0);

	// マップの描画機能を初期化
	mapRender_ = std::make_unique<MapRender>();
	int textureIndex = wallModel.materials[wallModel.materialIndex[0]].textureIndex;
	mapRender_->Initialize(drawDataManager_->GetDrawData(wallModel.drawDataIndex), textureIndex);
	mapRender_->SetConfig(currentMap_.currentMap.renderData);

	//Debug用マップ描画の初期化
	debugMapRender_ = std::make_unique<DebugMCRender>();
	debugMapRender_->Initialize(drawDataManager_->GetDrawData(wallModel.drawDataIndex));
	debugMapRender_->SetAlpha(1.0f);

	//装飾用オブジェクトの描画機能を初期化
	staticObjectRender_ = std::make_unique<StaticObjectRender>();
	staticObjectRender_->Initialize(modelManager_, drawDataManager_, false);
	staticObjectRender_->SetWallIndex(commonData_->newMapManager->GetWallIndex());
	staticObjectRender_->SetObjects(currentMap_.currentMap.decorations);

	//ColorMap作成
	LoadDebugColorMap();

	// MiniMapの初期化
	miniMap_ = std::make_unique<MiniMap>();
	DrawData planeData = drawDataManager_->GetDrawData(modelManager_->GetNodeModelData(1).drawDataIndex);
	DrawData visionFrame = drawDataManager_->GetDrawData(modelManager_->GetNodeModelData(modelManager_->LoadModel("VisionFrame")).drawDataIndex);
	miniMap_->Initialize((int)currentMap_.currentMap.mapChipData[0].size(), (int)currentMap_.currentMap.mapChipData.size(),
		commonData_->miniMapDisplay.get(), planeData, visionFrame, textureManager_);

	//================================================================
	// 鉱石システム
	//================================================================

	// フォント用
	auto draw = drawDataManager_->GetDrawData(modelManager_->GetNodeModelData(1).drawDataIndex);
	// spriteモデルを取得
	int sprModelID = modelManager_->LoadModel(spriteModelName);
	auto sprModel = modelManager_->GetNodeModelData(sprModelID);


	// 鉱石モデル
	int smallModelID = modelManager_->LoadModel("Small");
	auto smallModel = modelManager_->GetNodeModelData(smallModelID);
	int smallIndex = textureManager_->GetTexture("MineralDeposite_Small.png");

	int midleModelID = modelManager_->LoadModel("Middle");
	auto midleModel = modelManager_->GetNodeModelData(midleModelID);
	int midleIndex = textureManager_->GetTexture("MineralDeposite_Middle-0.png");

	int largeModelID = modelManager_->LoadModel("Large");
	auto largeModel = modelManager_->GetNodeModelData(largeModelID);
	int largeIndex = textureManager_->GetTexture("MineralDeposite_Large-0.png");

	// 演出モデル
	int boxModelID = modelManager_->LoadModel("BoxParticle");
	auto boxModel = modelManager_->GetNodeModelData(boxModelID);

	// spriteモデルを取得(アンカーポイントが違うスプライト)
	int hpModeID = modelManager_->LoadModel(unitHpModelName);
	auto hpMode = modelManager_->GetNodeModelData(hpModeID);

	int clockIconiTextureIndex = textureManager_->GetTexture("Clock.png");

	// 鉱石の管理システムを初期化
	oreItemManager_ = std::make_unique<OreItemManager>();
	// 描画データを設定ｓる
	oreItemManager_->SetModle(drawDataManager_->GetDrawData(smallModel.drawDataIndex), drawDataManager_->GetDrawData(midleModel.drawDataIndex), drawDataManager_->GetDrawData(largeModel.drawDataIndex),
		smallIndex, midleIndex, largeIndex);
	oreItemManager_->Initialize(drawDataManager_->GetDrawData(sprModel.drawDataIndex), drawDataManager_->GetDrawData(hpMode.drawDataIndex),
		fontName, draw, fontLoader_, drawDataManager_->GetDrawData(boxModel.drawDataIndex),
		commonData_->nextStageIndex, clockIconiTextureIndex);

	// 鉱石の回収ノルマを設定
	OreItemStorageNum::maxOreItemNum_ = currentMap_.norma;

	// マップシステムを取得
	oreItemManager_->SetMapChipField(mapChipField_.get());
	oreItemManager_->SetDirtParticleEmit();

	//鉱床の配置
	PutGold();

	//=============================================================================
	// 拠点システム
	//=============================================================================

	// 鉱石モデル
	int homeModelID = modelManager_->LoadModel(oreBaseModelName);
	auto homeModel = modelManager_->GetNodeModelData(homeModelID);

	// 鉱石のテクスチャを取得
	int homeIndex = textureManager_->GetTexture("OreBase-0.png");

	// spriteモデルを取得
	int spritModelID = modelManager_->LoadModel(spriteModelName);
	auto spritModel = modelManager_->GetNodeModelData(spritModelID);

	// 拠点管理クラス
	homeManager_ = std::make_unique<HomeManager>();
	homeManager_->Initialize(drawDataManager_->GetDrawData(homeModel.drawDataIndex), homeIndex, mapChipField_->GetHomePos());

	// 拠点のミニマップアイコン追加
	int homeIconIndex = textureManager_->GetTexture("OreBaseMapIcon_02.png");
	homeManager_->SetMinMapPos(miniMap_.get(), drawDataManager_->GetDrawData(spritModel.drawDataIndex), homeIconIndex);

	//============================================================================
	// ユニットシステム
	//============================================================================

	// プレイヤーモデルを取得
	int playerModelID = modelManager_->LoadModel(playerModelName);
	auto playerModel = modelManager_->GetNodeModelData(playerModelID);

	// プレイヤーのテクスチャを取得
	int playerTextureIndex = textureManager_->GetTexture("Player.png");

	// おれモデルを取得
	int oreModelID = modelManager_->LoadModel(oreModelName);
	auto oreModel = modelManager_->GetNodeModelData(oreModelID);

	// おれのテクスチャを取得
	int oreTextureIndex = textureManager_->GetTexture("untitled-0.png");

	// アイコン表示
	int oreMapIconIndex = textureManager_->GetTexture("OreMapIcon.png");
	int playerMapIconIndex = textureManager_->GetTexture("PlayerMapIcon.png");

	// ステージの番号を取得する
	int32_t sn = -1;
	if (commonData_->nextStageIndex >= 2) {
		sn = commonData_->stageCount-1;
	}

	// ユニットの管理クラス
	unitManager_ = std::make_unique<UnitManager>();
	unitManager_->Initalize(mapChipField_.get(),
		drawDataManager_->GetDrawData(playerModel.drawDataIndex), playerTextureIndex,
		drawDataManager_->GetDrawData(oreModel.drawDataIndex), oreTextureIndex,
		commonData_->keyManager.get(), playerInitPos, commonData_->oreNum,
		drawDataManager_->GetDrawData(spritModel.drawDataIndex), oreMapIconIndex, playerMapIconIndex,
		sn);

	// ミニマップを設定
	unitManager_->SetMinMap(miniMap_.get());

	// 拠点管理クラスを設定
	unitManager_->SetHomeManager(homeManager_.get());

	// 鉱石モデル
	int mItemModelID = modelManager_->LoadModel(mineralItemModelName);
	auto mItemModel = modelManager_->GetNodeModelData(mItemModelID);

	// 鉱石のテクスチャを取得
	int mItemTextureIndex = textureManager_->GetTexture("Mineral-0.png");

	// ユニットの演出管理クラス(仮で作成したため消すかも)
	unitEffectManager_ = std::make_unique<UnitEffectManager>();
	unitEffectManager_->Initialize(drawDataManager_->GetDrawData(mItemModel.drawDataIndex), mItemTextureIndex, drawDataManager_->GetDrawData(boxModel.drawDataIndex));

	// 演出管理クラスを取得
	unitManager_->SetUnitEffect(unitEffectManager_.get());

	//==========================================================================================
	// ポストエフェクト
	//==========================================================================================

	postEffect_ = std::make_unique<PostEffect>();
	postEffect_->Initialize(textureManager_, drawDataManager_->GetDrawData(commonData_->postEffectDrawDataIndex));

	postEffectConfig_.window = gameWindow_->GetWindow();
	postEffectConfig_.origin = display_;
	postEffectConfig_.jobs_ = UINT(PostEffectJob::Fade);

	fade_.alpha = 0.0f;
	fade_.color = { 0.0f,0.0f,0.0f };

	//==================================================================
	// UI
	//==================================================================

	// spriteモデルを取得
	int spriteModelID = modelManager_->LoadModel(spriteModelName);
	auto spriteModel = modelManager_->GetNodeModelData(spriteModelID);

	/// シーン遷移の処理
	fadeTransition_ = std::make_unique<FadeTransition>();
	fadeTransition_->Initialize(drawDataManager_->GetDrawData(spriteModel.drawDataIndex));

	auto drawData = drawDataManager_->GetDrawData(modelManager_->GetNodeModelData(1).drawDataIndex);

	// 星の画像
	int starTextureIndex = textureManager_->GetTexture("star.png");
	// 演出用の線
	int lineTextureIndex = textureManager_->GetTexture("lineEffect.png");
	// icon
	int oreItemTextureIndex = textureManager_->GetTexture("OreItem.png");
	int oreiTextureIndex = textureManager_->GetTexture("Ore.png");

	// ゲームのUI管理クラス
	gameUIManager_ = std::make_unique<GameUIManager>();
	gameUIManager_->Initialize(drawDataManager_->GetDrawData(spriteModel.drawDataIndex), starTextureIndex, lineTextureIndex, oreiTextureIndex, oreItemTextureIndex, clockIconiTextureIndex,
		fontName, drawData, fontLoader_,
		commonData_->stageCount);

	// spriteモデルを取得(アンカーポイントが違うスプライト)
	int uModelID = modelManager_->LoadModel(unitHpModelName);
	auto uModel = modelManager_->GetNodeModelData(uModelID);

	// ユニットのHpUI
	oreUnitHpUI_ = std::make_unique<OreUnitHPUI>();
	oreUnitHpUI_->Initialize(drawDataManager_->GetDrawData(uModel.drawDataIndex));
	// ユニット管理クラスに持たせる
	unitManager_->SetUnitHp(oreUnitHpUI_.get());

	// 登録
	RegisterDebugParam();
	ApplyDebugParam();

	// 時間を測る
	timeTracker_ = std::make_unique<TimeTracker>();
	timeTracker_->SetCountTime(mTime_, sTime_);

	// 時間管理クラスを取得
	oreItemManager_->SetTimeTracker(timeTracker_.get());

	// 最初と最後のカウントをするUI
	startCountUI_ = std::make_unique<StartCountUI>();
	startCountUI_->Initialize(fontName, drawData, fontLoader_, commonData_->stageCount, "RampartOne-Regular.ttf");
	startCountUI_->isStart_ = true;

	// ミニマップ操作時の時間操作処理
	miniMap_->SetOnClicked([this]() {
		if (miniMap_->PleasePose()) {
			timeTracker_->EndMeasureTimes();
			isActiveMinMap_ = true;

			// アイコン用にユニット位置を更新する
			unitManager_->MiniMapUpdate();
		} else {
			timeTracker_->StartMeasureTimes();
			isActiveMinMap_ = false;
		}
		});

	// 矢印の画像
	int arrowIndex = textureManager_->GetTexture("Arrow.png");
	int alertIndex = textureManager_->GetTexture("AlertIcon.png");
	int playerIconIndex = textureManager_->GetTexture("PlayerIcon.png");
	int outlineIconIndex = textureManager_->GetTexture("AlertIcon_02.png");

	// ユニットのUI管理
	unitMarkUIManager_ = std::make_unique<UnitMarkUIManager>();
	unitMarkUIManager_->Initialize(drawDataManager_->GetDrawData(spriteModel.drawDataIndex), arrowIndex, alertIndex, playerIconIndex, outlineIconIndex);
	// カメラを設定
	unitMarkUIManager_->SetCamera(cameraController_.get());
	// ミニマップを設定
	unitMarkUIManager_->SetMinMap(miniMap_.get());

	// ユニット管理クラスに登録する
	unitManager_->SetUnitMarkUI(unitMarkUIManager_.get());

	// 衝突ログ
	int unitConfliIndex = textureManager_->GetTexture("OreCrushNews.png");
	int logEffectIndex = textureManager_->GetTexture("logEffect.png");
	int unitDeathIndex = textureManager_->GetTexture("OreDeathNews.png");

	// ログUI
	logUI_ = std::make_unique<LogUI>();
	logUI_->Initialize(drawDataManager_->GetDrawData(spriteModel.drawDataIndex), unitConfliIndex, logEffectIndex, unitDeathIndex);

	// ユニット管理クラスにログUIシステムを設定
	unitManager_->SetLogUI(logUI_.get());

	//=======================================================
	// その他のシーンを初期化
	//=======================================================
	InitializeOtherScene();

	gameOverTask_ = std::make_unique<GameOverTask>();
	int modelIndex = modelManager_->LoadModel("GameOverTie");
	auto modelData = modelManager_->GetNodeModelData(modelIndex);
	textureIndex = textureManager_->GetTexture("GameOver_Tie1.png");
	gameOverTask_->Initialize(drawDataManager_->GetDrawData(modelData.drawDataIndex),
		textureIndex, &fade_.alpha);

	// 最初に一度だけ更新処理を呼ぶ
	InGameScene();

	auto logger = getLogger("Result");
	logger->info("=========================================================");
	logger->info("-- Stage {} --");
	logger->info("MaxOreNum: {}\tKillOreNum: {}\tGetOreNum: {}\tMaxGoldNum: {}\tSumGoldNum: {}\n", 
		commonData_->maxOreNum, commonData_->killOreNum, commonData_->getOreNum, commonData_->maxGoldNum, commonData_->sumGoldNum_);


	/// チュートリアル
	int guidTextureIndex = textureManager_->GetTexture("TutrialImage.png");

	//==============================================================================
	// 操作説明
	//==============================================================================

	// 操作UI
	guideSprite_ = std::make_unique<SpriteObject>();
	guideSprite_->Initialize(drawDataManager_->GetDrawData(spriteModel.drawDataIndex), { 1280,720.0f });
	guideSprite_->transform_.position = { 640.0f,360.0f,0.0f };
	guideSprite_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	guideSprite_->SetTexture(guidTextureIndex);
	guideSprite_->Update();


	if ((commonData_->nextStageIndex == 0) && commonData_->stageCount <= 1) {
		isOpenGuide_ = true;
	} else {
		isOpenGuide_ = false;
	}
}

void GameScene::InitializeOtherScene() {

	// spriteモデルを取得
	int spriteModelID = modelManager_->LoadModel(spriteModelName);
	auto spriteModel = modelManager_->GetNodeModelData(spriteModelID);

	auto drawData = drawDataManager_->GetDrawData(modelManager_->GetNodeModelData(1).drawDataIndex);

	// ゲームオーバーUIの初期化
	gameOverUI_ = std::make_unique<GameOverUI>();
	gameOverUI_->Initialize(drawDataManager_->GetDrawData(spriteModel.drawDataIndex), commonData_->keyManager.get(), fontName, drawData, fontLoader_);
	// リトライ
	gameOverUI_->SetOnRetryClicked([this]() {
		isSceneChange_ = true;
		});
	// 選択
	gameOverUI_->SetOnSelectClicked([this]() {
		isSceneChange_ = true;
		});

	// フロアクリアのテクスチャを取得
	int florClerTexIndex = textureManager_->GetTexture("FloorClear.png");
	// 星の画像
	int starTextureIndex = textureManager_->GetTexture("star.png");

	// クリアUIの初期化
	clearUI_ = std::make_unique<ClearUI>();
	clearUI_->Initialize(drawDataManager_->GetDrawData(spriteModel.drawDataIndex), commonData_->keyManager.get(), fontName, drawData, fontLoader_, hasNextMap_, florClerTexIndex, starTextureIndex);
	// リトライ
	clearUI_->SetOnRetryClicked([this]() {
		/*isSceneChange_ = true;*/

		hasNextMap_ = true;

		});
	// 選択
	clearUI_->SetOnSelectClicked([this]() {
		/*isSceneChange_ = true;*/

		hasNextMap_ = false;

		});

	// シーン遷移で仕様するアルファ値を取得
	clearUI_->SetSceneColor(&fade_.alpha);

	// 操作説明のテクスチャを取得
	int guidTextureIndex = textureManager_->GetTexture("TutrialImage.png");

	int pBackIndex = textureManager_->GetTexture("Pause_Back.png");
	int pPlayIndex = textureManager_->GetTexture("Pause_HowToPlay.png");
	int pSeleIndex = textureManager_->GetTexture("Pause_Select.png");
	int tabIndex = textureManager_->GetTexture("Tab_Pause.png");

	int plogIndex = textureManager_->GetTexture("Pebble_01.png");

	// ポーズシーンUIの初期化
	pauseUI_ = std::make_unique<PauseUI>();
	pauseUI_->Initialize(drawDataManager_->GetDrawData(spriteModel.drawDataIndex), static_cast<int32_t>(guidTextureIndex), commonData_->keyManager.get(),
		fontName, drawData, fontLoader_,
		pBackIndex, pPlayIndex, pSeleIndex, plogIndex, tabIndex);
	// リトライ
	pauseUI_->SetOnRetryClicked([this]() {
		isPauseScene_ = !isPauseScene_;

		if (isPauseScene_) {
			timeTracker_->EndMeasureTimes();
		} else {
			timeTracker_->StartMeasureTimes();
		}
		});
	// 選択
	pauseUI_->SetOnSelectClicked([this]() {
		isPauseScene_ = true;
		isSceneChange_ = true;
		selectSceneBack_ = true;
		});
}

std::unique_ptr<IScene> GameScene::Update() {

	//! ====================================================
	//! 更新処理まとめ
	//! ====================================================

	//いつでもやる共通の更新処理
	CommonUpdate();

	if (fadeTransition_->IsAnimation()) {

		// ゲーム開始時の遷移処理
		fadeTransition_->Update();

	} else {

		if (!isOpenGuide_) {

			if (!isPauseScene_ && startCountUI_->isStartAnimeEnd()) {

				// ミニマップの更新処理
				miniMap_->Update();

				if (!isActiveMinMap_) {
					// ゲームの更新処理
					InGameScene();
				} else {
					gameUIManager_->StopUpdate();
				}

				// 鉱石管理の更新処理
				oreItemManager_->Update(isActiveMinMap_);
			}

			if (!isGameOver_ && !isGameClear_ && startCountUI_->isStartAnimeEnd()) {
				// ポーズシーンの更新処理
				pauseUI_->Update();
			}
		} else {

			// 左クリック
			if ((Input::GetMouseButtonState()[0] & 0x80) && !(Input::GetPreMouseButtonState()[0] & 0x80)) {
				isOpenGuide_ = false;
			}
		}
	}

	//====================================================
	// ゲームシーンから遷移するときの処理
	//====================================================
	if (isGameOver_) {
		if (gameOverTask_->Update(FpsCount::deltaTime)) {
			SaveGameData();
			return std::make_unique<ResultScene>();
		}
	}

	if ( clearUI_->IsClearAnimationEnd()) {

		//以下シーン切り替えについて
		if (hasNextMap_) {
			SaveGameData();
			return std::make_unique<OreAddScene>();

		} else {
			SaveGameData();

			//終了処理(とりあえずの仮置き)
			return std::make_unique<ResultScene>();
		}
	}

	clearUI_->Update();

	//====================================================
	// クリア、ゲームオーバーの判定処理
	//====================================================
	// 時間切れ
	if (timeTracker_->isFinishd()) {

		// タイマーの終了音声を再生
		if (!isPlayTimeUpSH_) {
			isPlayTimeUpSH_ = true;
			AudioManager::GetInstance().Play(timeUpSH_, 0.5f, false);
		}

		// 鉱石が目標数納品を達成するか、鉱石がなくなればクリア
		if (OreItemStorageNum::currentOreItemNum_ >= OreItemStorageNum::maxOreItemNum_ ||
			oreItemManager_->GetCurrentOreItemNum() <= 0) {

			clearUI_->Start(true);
			//SaveGameData();

			// クリア音声を再生
			if (!isPlayClearSH_) {
				isPlayClearSH_ = true;
				AudioManager::GetInstance().Play(clearSH_, 0.5f, false);
			}

		} else {

			isGameOver_ = true;
			//SaveGameData();

		}
	}

	// 出撃出来るユニットがいなくなったらゲームオーバー
	if (unitManager_->GetMaxOreCount() <= 0) {
		// ゲームオーバーシーンに移動
		isGameOver_ = true;
		//SaveGameData();
	}

	//====================================================
	// ポーズシーンでselectシーンに戻るよう要求された場合
	//====================================================
	if (selectSceneBack_) {
		return std::make_unique<SelectScene>();
	}

	return nullptr;
}



void GameScene::CommonUpdate() {
#ifdef USE_IMGUI
	// 値の適応
	ApplyDebugParam();
#endif

#pragma region 外部入力系の情報受け取り
	// Δタイムを取得する
	FpsCount::deltaTime = engine_->GetFPSObserver()->GetDeltatime();

	// MiniMapがポーズを要求してきたらΔタイムを0にする
	if (miniMap_->PleasePose()) {
		//FpsCount::deltaTime = 0.0f;
	}

	// 入力処理の更新
	input_->Update();
	commonData_->keyManager->Update();

	debugCamera_->Update();
	camera_ = *static_cast<Camera*>(debugCamera_.get());

	// マウスのスクリーン座標を取得する
	POINT cursorPos;
	if (GetCursorPos(&cursorPos)) {
		// スクリーン座標をクライアント座標に変換
		ScreenToClient(gameWindow_->GetWindow()->GetHwnd(), &cursorPos);
		// カーソル位置をワールド座標に変換
		DebugMousePos::screenMousePos = { static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y) };
	}
#pragma endregion

	if (isOpenGuide_) { return; }

	//===========================================================
	// 時間計測処理
	//===========================================================
	// スタートアニメーションが終わった時、計測をスタートする
	if (!isTimerSet_ && startCountUI_->isStartAnimeEnd()) {
		timeTracker_->StartMeasureTimes();
		isTimerSet_ = true;
	}

	// 前の時間を取得
	gameUIManager_->SetPreTime(TimeLimit::totalSeconds);
	startCountUI_->SetPreTime(TimeLimit::totalSeconds);

	// 時間を更新
	timeTracker_->Update();


	//================================================
	// UIの更新処理
	//================================================

	// ゲームシーンのUIの更新処理
	gameUIManager_->Update(unitManager_->GetMaxOreCount() - unitManager_->GetOreCount(), unitManager_->GetMaxOreCount());

	// 開始と最後のカウントする更新処理
	startCountUI_->Update();

	// ログUIの更新処理
	logUI_->Update();

	// ==============================================================
	//  カメラの更新処理
	// ==============================================================
	cameraController_->SetTargetPos(unitManager_->GetPlayerPosition());
	// カメラの更新処理
	cameraController_->Update();

}

void GameScene::InGameScene() {

	//====================================================
	// カメラの更新処理
	//====================================================
	cameraController_->SetTargetPos(unitManager_->GetPlayerPosition());

	// カメラの更新処理
	cameraController_->Update();

	//==============================================================
	// ユニットの選択処理
	//==============================================================

	auto PlayReject = [this]() {
		if (!AudioManager::GetInstance().IsPlay(oreRejectedSH_)) {
			AudioManager::GetInstance().Play(oreRejectedSH_, 0.5f, false);
		}
		};

	auto PlayGo = [this]() {
		if (!AudioManager::GetInstance().IsPlay(oreGoSH_)) {
			AudioManager::GetInstance().Play(oreGoSH_, 0.5f, false);
		}
		};

	// マウスの位置に鉱石が存在していればユニットを動かす
	Vector3 oreWorldPos = {};
	if (oreItemManager_->IsSelectOre(cameraController_->GetWorldPos(), oreWorldPos) && !isInputPress_) {

		// 左クリックを取得
		if ((Input::GetMouseButtonState()[0] & 0x80) && !(Input::GetPreMouseButtonState()[0] & 0x80)) {
			// クリックアニメーションを開始
			cameraController_->StartAnimation();

			int32_t num = unitManager_->GetMaxOreCount() - unitManager_->GetOreCount();

			if (num > 0) {

				// 選択された鉱石を取得
				OreItem* selectedOreItem = oreItemManager_->GetOreItemForId();

				// 出撃させる数
				int32_t spawnNum = unitManager_->GetUnitSpawnNum();

				if (spawnNum > 0) {
					// 追加出来るかを確認
					int32_t deltaNum = selectedOreItem->IsFullWorker(spawnNum);
					if (deltaNum >= 0) {

						// おれを追加
						unitManager_->RegisterUnit(selectedOreItem->GetPos(), spawnNum, 0, selectedOreItem);

						// 鉱石側の労働者カウントを増やす
						for (int i = 0; i < spawnNum; ++i) {
							selectedOreItem->AddWorker();
						}

						if (spawnNum <= 0) {
							PlayReject();
						} else {
							PlayGo();
						}
					} else {
						if (spawnNum >= deltaNum * -1.0f) {
							// おれを追加
							unitManager_->RegisterUnit(selectedOreItem->GetPos(), spawnNum, deltaNum, selectedOreItem);

							// 鉱石側の労働者カウントを増やす
							int32_t actualSpawnCount = spawnNum + deltaNum;
							for (int i = 0; i < actualSpawnCount; ++i) {
								selectedOreItem->AddWorker();
							}
							if (actualSpawnCount <= 0) {
								PlayReject();
							} else {
								PlayGo();
							}
						}
					}
				} else {
					PlayReject();
				}
			} else {
				PlayReject();
				// これ以上配置することが出来ない時、演出を実行
				gameUIManager_->StartAnimation();
			}
		}
	} else {
		// 左クリックを取得
		if ((Input::GetMouseButtonState()[0] & 0x80) && !(Input::GetPreMouseButtonState()[0] & 0x80)) {

			// プレイヤーの移動位置を設定
			unitManager_->MovePlayer(cameraController_->GetWorldPos());

			// クリックアニメーションを開始
			cameraController_->StartAnimation();
		}

		// 左クリック長押しの場合
		if (Input::GetMouseButtonState()[0] & 0x80) {

			pressTimer_ += FpsCount::deltaTime / 0.5f;

			if (pressTimer_ >= 1.0f) {
				isInputPress_ = true;
				unitManager_->PressMovePlayer(cameraController_->GetWorldPos());
			}
		} else {
			pressTimer_ = 0.0f;
			isInputPress_ = false;
		}
	}

	//===================================================
	// 拠点の更新処理
	//===================================================

	// 拠点の更新処理
	homeManager_->Update();

	//====================================================================
	// ユニットの更新処理
	//====================================================================

	// ユニットの更新処理
	unitManager_->Update();

	//============================================
	// 当たり判定
	//============================================

	// 全ての当たり判定を判定
	colliderManager_->CollisionCheckAll();


	// 衝突判定あとの演出更新処理
	unitEffectManager_->PostCollisionUpdate();
}

void GameScene::Draw() {

	for (int i = 0; i < 2; ++i) {
		Matrix4x4 vpMatrix{};

		if (i == 0) {
			vpMatrix = miniMap_->PreDraw(gameWindow_->GetWindow())->GetVPMatrix();
		} else {
			display_->PreDraw(gameWindow_->GetCommandObject(), true);
			vpMatrix = cameraController_->GetVPMatrix();
		}

		// カメラ行列
		LightManager::GetInstance()->SetCameraPos(cameraController_->GetCameraWorldPos());

		// マップを描画
		mapRender_->Draw(vpMatrix, gameWindow_->GetWindow());

		// 装飾オブジェクトの描画
		staticObjectRender_->Draw(vpMatrix, gameWindow_->GetWindow());

		// デバッグ用マップ描画
		//debugMapRender_->Draw(vpMatrix, colorMap_, currentMap_.currentMap.mapChipData, gameWindow_->GetWindow());

		// 鉱石の描画
		oreItemManager_->Draw(gameWindow_->GetWindow(), vpMatrix);

		// ユニットの拠点を描画
		homeManager_->Draw(gameWindow_->GetWindow(), vpMatrix);

		// ユニットを描画
		unitManager_->Draw(gameWindow_->GetWindow(), vpMatrix);

		// ユニットの演出を描画
		unitEffectManager_->Draw(gameWindow_->GetWindow(), vpMatrix);

		// ユニットのHp描画
		oreUnitHpUI_->Draw(gameWindow_->GetWindow(), vpMatrix);

		// 鉱石の破片パーティクルを描画
		oreItemManager_->DrawEffect(gameWindow_->GetWindow(), vpMatrix);

		// マウスのクリックアニメーション
		if (!isActiveMinMap_) {
			cameraController_->DrawAnimation(gameWindow_->GetWindow(), vpMatrix);
		}

		Matrix4x4 vpMatrix2d;

		//MiniMap
		if (i == 1) {

			//============================================================================
			// 2D描画
			//============================================================================

			/// UIの描画処理
			vpMatrix2d = uiCamera_->GetVPMatrix();

			miniMap_->Draw(gameWindow_->GetWindow());

			// ユニットのUIマークを描画
			if (!miniMap_->PleasePose()) {
				unitMarkUIManager_->DrawUI(gameWindow_->GetWindow(), vpMatrix2d);
			}

			// 開始と終わりのカウントの描画
			startCountUI_->Draw(gameWindow_->GetWindow(), vpMatrix2d);

			// ログのUIを描画
			logUI_->Draw(gameWindow_->GetWindow(), vpMatrix2d);

			if (startCountUI_->isStartAnimeEnd() ){

				// ポーズシーンを描画
				pauseUI_->Draw(gameWindow_->GetWindow(), vpMatrix2d);

				// ミニマップ
				if (miniMap_->PleasePose()) {
					// 拠点アイコン
					homeManager_->DrawIcon(gameWindow_->GetWindow(), vpMatrix2d);
					// ユニットアイコンを描画
					unitManager_->DrawIcon(gameWindow_->GetWindow(), vpMatrix2d);

					// 衝突位置アイコンを描画
					unitMarkUIManager_->DrawUI(gameWindow_->GetWindow(), vpMatrix2d);
				} else {
					
					// 時間追加UIを描画
					oreItemManager_->DrawTimeUI(gameWindow_->GetWindow(), vpMatrix2d);
				}

				// ゲームのUIを描画
				gameUIManager_->Draw(gameWindow_->GetWindow(), vpMatrix2d, !miniMap_->PleasePose());

				// 操作UIを表示
				pauseUI_->DrawGuideUI(gameWindow_->GetWindow(), vpMatrix2d);
			}

			// ポーズシーンを描画
			//pauseUI_->Draw(gameWindow_->GetWindow(), vpMatrix2d);

			// ゲームオーバーシーンの描画処理
			if (isGameOver_) {
				gameOverTask_->Draw(gameWindow_->GetWindow(), vpMatrix2d);
			}

			// クリアシーンの描画処理
			if (isGameClear_) {

			}

			// クリアシーンの更新処理
			clearUI_->Draw(gameWindow_->GetWindow(), vpMatrix2d);



			// 操作説明
			if (isOpenGuide_) {
				guideSprite_->Draw(gameWindow_->GetWindow(), vpMatrix2d);
			}


			// シーン遷移の描画
			fadeTransition_->Draw(gameWindow_->GetWindow(), vpMatrix2d);

		} else {
			Vector3 playerPos = unitManager_->GetPlayerPosition();
			float range = cameraController_->GetRange();
			miniMap_->PostDraw(gameWindow_->GetWindow(), vpMatrix, playerPos, range);
		}
	}

	display_->PostDraw(gameWindow_->GetCommandObject());

	//PostEffectとか
#ifdef SH_RELEASE
	postEffectConfig_.output = gameWindow_->GetDualDisplay();
#endif
	postEffect_->CopyBuffer(PostEffectJob::Fade, fade_);
	postEffect_->Draw(postEffectConfig_);

#ifdef SH_RELEASE
	gameWindow_->PreDraw(false);
#else
	gameWindow_->PreDraw(true);
#endif

#ifdef USE_IMGUI

	ImGui::Begin("ScoreDebug");

	ImGui::Text("MaxOre: %d, KillOre: %d", commonData_->maxOreNum, commonData_->killOreNum);
	ImGui::Text("GetOre: %d", commonData_->getOreNum);
	ImGui::Text("SumGold: %d, MaxGold: %d", commonData_->sumGoldNum_, commonData_->maxGoldNum);

	ImGui::End();
	
#endif

	//ImGui
	miniMap_->DrawImGui();
	gameWindow_->DrawDisplayWithImGui();
	paramManager_->Draw();
	LightManager::GetInstance()->DrawImGui();
	gameOverTask_->DrawImGui();

	// カメラのデバック情報
	cameraController_->DebugDraw();

	// ユニット
	unitManager_->DrawUI();

	// 鉱石
	oreItemManager_->DrawUI();


	engine_->ImGuiDraw();
}

void GameScene::RegisterDebugParam() {
	// ゲームの時間
	GameParamEditor::GetInstance()->AddItem("GameTime", "m", mTime_);
	GameParamEditor::GetInstance()->AddItem("GameTime", "s", sTime_);

	// 鉱石数
	GameParamEditor::GetInstance()->AddItem("OreItem", "MaxOreItem", currentMap_.norma);
}

void GameScene::ApplyDebugParam() {
	// ゲームの時間
	mTime_ = GameParamEditor::GetInstance()->GetValue<float>("GameTime", "m");
	sTime_ = GameParamEditor::GetInstance()->GetValue<float>("GameTime", "s");

	// 鉱石数
	//OreItemStorageNum::maxOreItemNum_ = GameParamEditor::GetInstance()->GetValue<int32_t>("OreItem", "MaxOreItem");
}

void GameScene::LoadDebugColorMap() {
	auto values = binaryManager_.Read("MapTypeEditorConfig");

	//colorMapの初期化
	for (int i = 0; i < int(TileType::Count); ++i) {
		Vector4 color = ConvertColor(lerpColor(0xff5500ff, 0x0055ffff, float(i) / (float(TileType::Count) - 1)));
		colorMap_[static_cast<TileType>(i)] = { color.x, color.y, color.z };
	}

	int index = 0;
	while (index < values.size()) {
		int tile = binaryManager_.Reverse<int>(values[index++].get());
		Vector3 color = binaryManager_.Reverse<Vector3>(values[index++].get());
		colorMap_[static_cast<TileType>(tile)] = color;
	}
}

void GameScene::PutGold() {
	MapChipData& data = currentMap_.currentMap.mapChipData;
	for (auto& row : data) {
		for (auto& tile : row) {

			OreType type = OreType::Small;

			switch (tile) {
			case TileType::Gold:
				type = OreType::Small;
				break;
			case TileType::MediumGold:
				type = OreType::Medium;
				break;
			case TileType::LargeGold:
				type = OreType::Large;
				break;
			default:
				type = OreType::MaxCount;
			}

			if (type == OreType::MaxCount) {
				continue;
			}

			Vector3 pos = {
				static_cast<float>(&tile - &row[0]),
				0.0f,
				static_cast<float>(&row - &currentMap_.currentMap.mapChipData[0])
			};
			oreItemManager_->AddOreItem(type, pos);

			tile = TileType::Gold;
		}
	}
}

Vector3 GameScene::GetPlayerInitPosition() {
	for (auto& row : currentMap_.currentMap.mapChipData) {
		for (auto& tile : row) {
			if (tile == TileType::PlayerSpawn) {
				tile = TileType::Road;
				return {
					static_cast<float>(&tile - &row[0]),
					0.0f,
					static_cast<float>(&row - &currentMap_.currentMap.mapChipData[0])
				};
			}
		}
	}

	return { 3.0f,0.0f,3.0f };
}

void GameScene::SaveGameData() {
	int goldNum = OreItemStorageNum::currentOreItemNum_;
	int oreNum = unitManager_->GetMaxOreCount();
	int norma = currentMap_.norma;
	commonData_->normaAndScore_.push_back({ norma, goldNum });
	commonData_->sumGoldNum_ += goldNum;
	commonData_->maxGoldNum = std::max(commonData_->maxGoldNum, goldNum);
	commonData_->maxOreNum = std::max(commonData_->maxOreNum, oreNum);
	commonData_->killOreNum += commonData_->oreNum - oreNum;
	commonData_->oreNum = oreNum;
	commonData_->goldNum = goldNum;
}
