#include "GameScene.h"
#include <Common/DebugParam/GameParamEditor.h>
#include"FpsCount.h"
#include"GameCamera/DebugMousePos.h"
#include"Item/OreItemStorageNum.h"
#include"LightManager.h"
#include <Utility/Easing.h>
#include<Game/SelectScene.h>

namespace {
	//MiniMap確認用
	bool poseMode = false;

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
	isSceneChange_ = false;
	isClearScene_ = false;
	isGameOverScene_ = false;
	OreItemStorageNum::currentOreItemNum_ = 0;
}

void GameScene::Initialize() {
	gameWindow_ = commonData_->mainWindow.get();
	display_ = commonData_->display.get();
	//カメラの初期化
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
	//パラメーター管理の初期化
	paramManager_ = std::make_unique<ParamManager>();

	//　当たり判定管理クラスを初期化
	colliderManager_ = std::make_unique<ColliderManager>();
	// 当たり判定管理クラスを登録
	Collider::SetColliderManager(colliderManager_.get());

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

	// ライトの初期化
	LightManager::light_.color = { 1.0f,1.0f,1.0f,1.0f };
	LightManager::light_.direction = { 0.0f,-1.0f,0.0f };
	LightManager::light_.intensity = 1.0f;

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

	currentMap_ = commonData_->newMapManager->GetStageMap(commonData_->nextStageIndex, commonData_->nextMapIndex);
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
	mapRender_->Initialize(drawDataManager_->GetDrawData(wallModel.drawDataIndex));
	mapRender_->SetConfig(currentMap_.currentMap.renderData);

	//Debug用マップ描画の初期化
	debugMapRender_ = std::make_unique<DebugMCRender>();
	debugMapRender_->Initialize(drawDataManager_->GetDrawData(wallModel.drawDataIndex));
	debugMapRender_->SetAlpha(1.0f);

	//装飾用オブジェクトの描画機能を初期化
	staticObjectRender_ = std::make_unique<StaticObjectRender>();
	staticObjectRender_->Initialize(modelManager_, drawDataManager_, false);
	staticObjectRender_->SetObjects(currentMap_.currentMap.decorations);

	//ColorMap作成
	LoadDebugColorMap();

	// MiniMapの初期化
	miniMap_ = std::make_unique<MiniMap>();
	DrawData planeData = drawDataManager_->GetDrawData(modelManager_->GetNodeModelData(1).drawDataIndex);
	DrawData visionFrame = drawDataManager_->GetDrawData(modelManager_->GetNodeModelData(modelManager_->LoadModel("VisionFrame")).drawDataIndex);
	miniMap_->Initialize((int)currentMap_.currentMap.mapChipData[0].size(), (int)currentMap_.currentMap.mapChipData.size(),
		textureManager_, planeData, visionFrame);

	//================================================================
	// 鉱石システム
	//================================================================

	// 鉱石モデル
	int oreItemModelID = modelManager_->LoadModel(mineralModelName);
	auto oreItemModel = modelManager_->GetNodeModelData(oreItemModelID);

	// 鉱石のテクスチャを取得
	int oreItemTextureIndex = textureManager_->GetTexture("MineralDeposite_02.png");

	// フォント用
	auto draw = drawDataManager_->GetDrawData(modelManager_->GetNodeModelData(1).drawDataIndex);
	// spriteモデルを取得
	int sprModelID = modelManager_->LoadModel(spriteModelName);
	auto sprModel = modelManager_->GetNodeModelData(sprModelID);

	// 鉱石の管理システムを初期化
	oreItemManager_ = std::make_unique<OreItemManager>();
	oreItemManager_->Initialize(drawDataManager_->GetDrawData(oreItemModel.drawDataIndex), oreItemTextureIndex,
		drawDataManager_->GetDrawData(sprModel.drawDataIndex),
		fontName, draw, fontLoader_);

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

	// 拠点管理クラス
	homeManager_ = std::make_unique<HomeManager>();
	homeManager_->Initialize(drawDataManager_->GetDrawData(homeModel.drawDataIndex), homeIndex, mapChipField_->GetHomePos());

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

	// ユニットの管理クラス
	unitManager_ = std::make_unique<UnitManager>();
	unitManager_->Initalize(mapChipField_.get(),
		drawDataManager_->GetDrawData(playerModel.drawDataIndex), playerTextureIndex,
		drawDataManager_->GetDrawData(oreModel.drawDataIndex), oreTextureIndex,
		commonData_->keyManager.get(), playerInitPos);

	// 拠点管理クラスを設定
	unitManager_->SetHomeManager(homeManager_.get());

	// 鉱石モデル
	int mItemModelID = modelManager_->LoadModel(mineralItemModelName);
	auto mItemModel = modelManager_->GetNodeModelData(mItemModelID);

	// 鉱石のテクスチャを取得
	int mItemTextureIndex = textureManager_->GetTexture("Mineral-0.png");

	// ユニットの演出管理クラス(仮で作成したため消すかも)
	unitEffectManager_ = std::make_unique<UnitEffectManager>();
	unitEffectManager_->Initialize(drawDataManager_->GetDrawData(mItemModel.drawDataIndex), mItemTextureIndex);

	// 演出管理クラスを取得
	unitManager_->SetUnitEffect(unitEffectManager_.get());

	//==========================================================================================
	// ポストエフェクト
	//==========================================================================================

	postEffect_ = std::make_unique<PostEffect>();
	postEffect_->Initialize(textureManager_, drawDataManager_->GetDrawData(commonData_->postEffectDrawDataIndex));

	postEffectConfig_.window = gameWindow_->GetWindow();
	postEffectConfig_.origin = display_;
	postEffectConfig_.jobs_ = 0;

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

	// ゲームのUI管理クラス
	gameUIManager_ = std::make_unique<GameUIManager>();
	gameUIManager_->Initialize(drawDataManager_->GetDrawData(spriteModel.drawDataIndex), fontName, drawData, fontLoader_);

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

	// 最初と最後のカウントをするUI
	startCountUI_ = std::make_unique<StartCountUI>();
	startCountUI_->Initialize(fontName, drawData, fontLoader_);
	startCountUI_->isStart_ = true;

	//=======================================================
	// その他のシーンを初期化
	//=======================================================
	InitializeOtherScene();

	// 最初に一度だけ更新処理を呼ぶ
	InGameScene();
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
		isRetry_ = true;
		});
	// 選択
	gameOverUI_->SetOnSelectClicked([this]() {
		isSceneChange_ = true;
		isRetry_ = false;
		});

	// クリアUIの初期化
	clearUI_ = std::make_unique<ClearUI>();
	clearUI_->Initialize(drawDataManager_->GetDrawData(spriteModel.drawDataIndex), commonData_->keyManager.get(), fontName, drawData, fontLoader_);
	// リトライ
	clearUI_->SetOnRetryClicked([this]() {
		isSceneChange_ = true;
		isRetry_ = true;
		});
	// 選択
	clearUI_->SetOnSelectClicked([this]() {
		isSceneChange_ = true;
		isRetry_ = false;
		});

	// 操作説明のテクスチャを取得
	int guidTextureIndex = textureManager_->GetTexture("TutrialImage.png");

	// ポーズシーンUIの初期化
	pauseUI_ = std::make_unique<PauseUI>();
	pauseUI_->Initialize(drawDataManager_->GetDrawData(spriteModel.drawDataIndex), static_cast<int32_t>(guidTextureIndex), commonData_->keyManager.get(), fontName, drawData, fontLoader_);
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
		});
}

std::unique_ptr<IScene> GameScene::Update() {
#ifdef USE_IMGUI
	// 値の適応
	ApplyDebugParam();
#endif

	// Δタイムを取得する
	FpsCount::deltaTime = engine_->GetFPSObserver()->GetDeltatime();

	// MiniMapがポーズを要求してきたらΔタイムを0にする
	if (miniMap_->PleasePose()) {
		FpsCount::deltaTime = 0.0f;
	}

	// 入力処理の更新
	input_->Update();
	commonData_->keyManager->Update();

	debugCamera_->Update();
	camera_ = *static_cast<Camera*>(debugCamera_.get());

	//===========================================================
	// 時間計測処理
	//===========================================================

	// Δタイムを取得する
	FpsCount::deltaTime = engine_->GetFPSObserver()->GetDeltatime();

	// スタートアニメーションが終わった時、計測をスタートする
	if (!isTimerSet_ && startCountUI_->isStartAnimeEnd()) {
		timeTracker_->StartMeasureTimes();
		isTimerSet_ = true;
	}

	//=============================================================
	// シーン遷移、シーンの管理
	//=============================================================

	// 演出が終わったので次のシーンに切り替える
	if (fadeTransition_->IsFinished()) {

	}

	if (fadeTransition_->IsAnimation()) {

		// シーン遷移
		fadeTransition_->Update();
	} else {

		if (!isGameOverScene_ && !isClearScene_ && startCountUI_->isStartAnimeEnd()) {
			// ポーズシーンの更新処理
			pauseUI_->Update();
		}

		if (isGameOverScene_ || isClearScene_) {

			if (isClearScene_) {
				// クリアシーンの更新処理
				clearUI_->Update();
			} else {
				// ゲームオーバーの更新処理
				gameOverUI_->Update();
			}
		} else {
			if (!isPauseScene_ && startCountUI_->isStartAnimeEnd()) {
				// ゲームの更新処理
				InGameScene();
			}
		}
	}

	//====================================================
	// 時間の更新処理
	//====================================================

	// 前の時間を取得
	gameUIManager_->SetPreTime(TimeLimit::totalSeconds);
	startCountUI_->SetPreTime(TimeLimit::totalSeconds);

	// 時間を更新
	timeTracker_->Update();

	// 時間切れになれば
	if (timeTracker_->isFinishd()) {

		// 鉱石が目標数納品を達成するか、鉱石がなくなればクリア
		if (OreItemStorageNum::currentOreItemNum_ >= OreItemStorageNum::maxOreItemNum_ ||
			oreItemManager_->GetCurrentOreItemNum() <= 0) {
			// クリアシーンに移動
			isClearScene_ = true;
		} else {
			// ゲームオーバーシーンに移動
			isGameOverScene_ = true;
		}
	}

	//================================================
	// UIの更新処理
	//================================================

	// ゲームシーンのUIの更新処理
	gameUIManager_->Update(unitManager_->GetMaxOreCount() - unitManager_->GetOreCount(), unitManager_->GetMaxOreCount());

	// 開始と最後のカウントする更新処理
	startCountUI_->Update();

	//================================================
	// シーンの切り替え
	//================================================

	// 切り替える
	if (isSceneChange_) {
		if (isRetry_) {
			return std::make_unique<GameScene>();
		} else {
			return std::make_unique<SelectScene>();
		}
	}

	return nullptr;
}

void GameScene::InGameScene() {

	//====================================================
	// カメラの更新処理
	//====================================================
	cameraController_->SetTargetPos(unitManager_->GetPlayerPosition());

	// マウスのスクリーン座標を取得する
	POINT cursorPos;
	if (GetCursorPos(&cursorPos)) {
		// スクリーン座標をクライアント座標に変換
		ScreenToClient(gameWindow_->GetWindow()->GetHwnd(), &cursorPos);
		// カーソル位置をワールド座標に変換
		DebugMousePos::screenMousePos = { static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y) };
	}
	// カメラの更新処理
	cameraController_->Update();

	//====================================================
	// ミニマップの更新処理
	//====================================================
	miniMap_->Update();
  
	//==============================================================
	// ユニットの選択処理
	//==============================================================

	// マウスの位置に鉱石が存在していればユニットを動かす
	Vector3 oreWorldPos = {};
	if (oreItemManager_->IsSelectOre(cameraController_->GetWorldPos(), oreWorldPos)) {

		// 左クリックを取得
		if ((Input::GetMouseButtonState()[0] & 0x80) && !(Input::GetPreMouseButtonState()[0] & 0x80)) {
			// クリックアニメーションを開始
			cameraController_->StartAnimation();

			int32_t num = unitManager_->GetMaxOreCount() - unitManager_->GetOreCount();

			if (num > 0) {

				// 選択された鉱石を取得
				OreItem* selectedOreItem = oreItemManager_->GetOreItemForId();

				// 追加出来るかを確認
				int32_t deltaNum = selectedOreItem->IsFullWorker(unitManager_->GetUnitSpawnNum());
				if (deltaNum >= 0) {

					// おれを追加
					unitManager_->RegisterUnit(selectedOreItem->GetPos(), 0, selectedOreItem);

					// 鉱石側の労働者カウントを増やす
					for (int i = 0; i < unitManager_->GetUnitSpawnNum(); ++i) {
						selectedOreItem->AddWorker();
					}
				} else {
					if (unitManager_->GetUnitSpawnNum() >= deltaNum * -1.0f) {
						// おれを追加
						unitManager_->RegisterUnit(selectedOreItem->GetPos(), deltaNum, selectedOreItem);

						// 鉱石側の労働者カウントを増やす
						int32_t actualSpawnCount = unitManager_->GetUnitSpawnNum() + deltaNum;
						for (int i = 0; i < actualSpawnCount; ++i) {
							selectedOreItem->AddWorker();
						}
					}
				}
			}
		}
	} else {
		// 左クリックを取得
		if ((Input::GetMouseButtonState()[0] & 0x80) && !(Input::GetPreMouseButtonState()[0] & 0x80)) {

			// クリックアニメーションを開始
			cameraController_->StartAnimation();
		}
	}

	//=====================================================
	// 鉱石の更新処理
	//=====================================================

	// 鉱石管理の更新処理
	oreItemManager_->Update();

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

	// 出撃出来るユニットがいなくなったらゲームオーバー
	if (unitManager_->GetMaxOreCount() <= 0) {
		// ゲームオーバーシーンに移動
		isGameOverScene_ = true;
	}

	//============================================
	// 当たり判定
	//============================================

	// 全ての当たり判定を判定
	colliderManager_->CollisionCheckAll();
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
		LightManager::light_.cameraWorldPos = cameraController_->GetCameraWorldPos();

		// マップを描画
		mapRender_->Draw(vpMatrix, gameWindow_->GetWindow());

		// 装飾オブジェクトの描画
		staticObjectRender_->Draw(vpMatrix, gameWindow_->GetWindow());

		// デバッグ用マップ描画
		debugMapRender_->Draw(vpMatrix, colorMap_, currentMap_.currentMap.mapChipData, gameWindow_->GetWindow());

		// 鉱石の描画
		oreItemManager_->Draw(gameWindow_->GetWindow(), vpMatrix);

		// ユニットの拠点を描画
		homeManager_->Draw(gameWindow_->GetWindow(), vpMatrix);

		// ユニットを描画
		unitManager_->Draw(gameWindow_->GetWindow(), vpMatrix);

		// ユニットの演出を描画(仮で作成したクラスなので消すかも)
		unitEffectManager_->Draw(gameWindow_->GetWindow(), vpMatrix);

		// ユニットのHp描画
		oreUnitHpUI_->Draw(gameWindow_->GetWindow(), vpMatrix);

		// マウスのクリックアニメーション
		cameraController_->DrawAnimation(gameWindow_->GetWindow(), vpMatrix);

		Matrix4x4 vpMatrix2d;

		//MiniMap
		if (i == 1) {

			//============================================================================
			// 2D描画
			//============================================================================

			/// UIの描画処理
			vpMatrix2d = uiCamera_->GetVPMatrix();

		// 開始と終わりのカウントの描画
		startCountUI_->Draw(gameWindow_->GetWindow(), vpMatrix);

		// ポーズシーンを描画
		pauseUI_->Draw(gameWindow_->GetWindow(), vpMatrix);

			// ゲームのUIを描画
			gameUIManager_->Draw(gameWindow_->GetWindow(), vpMatrix2d);

			// ポーズシーンを描画
			pauseUI_->Draw(gameWindow_->GetWindow(), vpMatrix2d);

			// ゲームオーバーシーンの描画処理
			if (isGameOverScene_) {
				// UIの更新処理
				gameOverUI_->Draw(gameWindow_->GetWindow(), vpMatrix2d);
			}

			// クリアシーンの描画処理
			if (isClearScene_) {
				// クリアシーンの更新処理
				clearUI_->Draw(gameWindow_->GetWindow(), vpMatrix2d);
			}

			// シーン遷移の描画
			fadeTransition_->Draw(gameWindow_->GetWindow(), vpMatrix2d);

			miniMap_->Draw(gameWindow_->GetWindow());

		} else {
			Vector3 playerPos = unitManager_->GetPlayerPosition();
			float range = cameraController_->GetRange();
			miniMap_->PostDraw(gameWindow_->GetWindow(), vpMatrix, playerPos, range);
		}
	}

	display_->PostDraw(gameWindow_->GetCommandObject());

	//PostEffectとか
	postEffectConfig_.output = gameWindow_->GetDualDisplay();
	postEffect_->Draw(postEffectConfig_);

	gameWindow_->PreDraw(false);


	//ImGui
	miniMap_->DrawImGui();
	gameWindow_->DrawDisplayWithImGui();
	paramManager_->Draw();

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
	OreItemStorageNum::maxOreItemNum_ = GameParamEditor::GetInstance()->GetValue<int32_t>("OreItem", "MaxOreItem");
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
