#include"SelectScene.h"
#include <Common/DebugParam/GameParamEditor.h>
#include"FpsCount.h"
#include"GameCamera/DebugMousePos.h"
#include"Utility/Easing.h"
#include<numbers>
#include"LightManager.h"
#include"SelectStageNum.h"
#include <Game/GameScene.h>
#include"Assets/Audio/AudioManager.h"
#include"RandomGenerator.h"

namespace {
	//Minimap確認用
	bool poseMode = false;

	std::string fontName = "YDWbananaslipplus.otf";

	std::string playerModelName = "Player";

	std::string spriteModelName = "Sprite";
}

SelectScene::~SelectScene() {
	// BGMを止める
	if (AudioManager::GetInstance().IsPlay(selectSH_)) {
		AudioManager::GetInstance().Stop(selectSH_);
	}
}

void SelectScene::Initialize() {
	LightManager::GetInstance()->Initialize();
	gameWindow_ = commonData_->mainWindow.get();
	display_ = commonData_->display.get();
	//カメラの初期化
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
	//パラメーター管理の初期化
	paramManager_ = std::make_unique<ParamManager>();

	postEffect_ = std::make_unique<PostEffect>();
	postEffect_->Initialize(textureManager_, drawDataManager_->GetDrawData(commonData_->postEffectDrawDataIndex));

	postEffectConfig_.window = gameWindow_->GetWindow();
	postEffectConfig_.origin = display_;
	postEffectConfig_.jobs_ = 0;

	// BGMを発生
	selectSH_ = AudioManager::GetInstance().GetHandleByName("SelectBGM.mp3");
	AudioManager::GetInstance().Play(selectSH_, 0.5f, true);

	// 決定音を取得
	decideSH_ = AudioManager::GetInstance().GetHandleByName("Decide.mp3");

	playerDesicion_ = AudioManager::GetInstance().GetHandleByName("SelectWalk.mp3");
	// 移動
	playerMove_ = AudioManager::GetInstance().GetHandleByName("SelectTranstate.mp3");

	SelectStageNum::num_ = 1;

	// UIカメラを設定
	uiCamera_ = std::make_unique<Camera>();
	uiCamera_->SetProjectionMatrix(OrthographicDesc{});
	uiCamera_->MakeMatrix();

	// 追従カメラ
	selectCamera_ = std::make_unique<SelectCamera>();
	selectCamera_->Initialize();

	// spriteモデルを取得
	int spriteModelID = modelManager_->LoadModel(spriteModelName);
	auto spriteModel = modelManager_->GetNodeModelData(spriteModelID);

	auto drawData = drawDataManager_->GetDrawData(modelManager_->GetNodeModelData(1).drawDataIndex);

	int selcIndex = textureManager_->GetTexture("SelectBG.png");

	// ステージのロゴ表示機能
	selectStageUI_ = std::make_unique<SelectStageUI>();
	selectStageUI_->Init(drawDataManager_->GetDrawData(spriteModel.drawDataIndex), fontName, drawData, fontLoader_, selcIndex);

	// プレイヤーモデルを取得
	int stageModelID = modelManager_->LoadModel(playerModelName);
	auto stageModel = modelManager_->GetNodeModelData(stageModelID);

	// ステージオブジェクトを描画
	for (int i = 0; i < stagePointObjects_.size(); ++i) {
		stagePointObjects_[i] = std::make_unique<DefaultObject>();
		stagePointObjects_[i]->Initialize(drawDataManager_->GetDrawData(stageModel.drawDataIndex),0);
		stagePointObjects_[i]->transform_.position = { i * 6.0f,-1.8f,0.0f };
		stagePointObjects_[i]->transform_.scale = { 1.5f,0.2f,1.5f };
		stagePointObjects_[i]->material_.color = { 0.4f,0.4f,0.4f,1.0f };
		stagePointObjects_[i]->Update();
	}

	// 洞窟
	int cave1ModelID = modelManager_->LoadModel("Cave1");
	auto cave1Model = modelManager_->GetNodeModelData(cave1ModelID);

	int cave2ModelID = modelManager_->LoadModel("Cave2");
	auto cave2Model = modelManager_->GetNodeModelData(cave2ModelID);

	int cave3ModelID = modelManager_->LoadModel("Cave3");
	auto cave3Model = modelManager_->GetNodeModelData(cave3ModelID);

	// 洞窟テクスチャ
	int cave1Index = textureManager_->GetTexture("Cave_01.png");
	int cave2Index = textureManager_->GetTexture("Cave_02.png");
	int cave3Index = textureManager_->GetTexture("Cave_03.png");

	// 洞窟オブジェクトを描画
	for (int i = 0; i < caveObjects_.size(); ++i) {
		caveObjects_[i] = std::make_unique<DefaultObject>();
		if (i == 0) {
			caveObjects_[i]->Initialize(drawDataManager_->GetDrawData(cave1Model.drawDataIndex), cave1Index);
		} else if (i == 1) {
			caveObjects_[i]->Initialize(drawDataManager_->GetDrawData(cave2Model.drawDataIndex), cave2Index);
		} else {
			caveObjects_[i]->Initialize(drawDataManager_->GetDrawData(cave3Model.drawDataIndex), cave3Index);
		}
		caveObjects_[i]->transform_.position = { i * 6.0f - 1.0f,3.0f,5.5f };
		caveObjects_[i]->transform_.scale = { 1.0f,1.0f,1.0f };
		caveObjects_[i]->material_.color = { 1.0f,1.0f,1.0f,1.0f };
		caveObjects_[i]->material_.isActive = true;
		caveObjects_[i]->Update();
	}

	// 足場オブジェクト
	int selectModelID = modelManager_->LoadModel("Select");
	auto selectModel = modelManager_->GetNodeModelData(selectModelID);

	// 足場テクスチャ
	int selectIndex = textureManager_->GetTexture("Select-0.png");

	// 足場オブジェクト
	floorObject_ = std::make_unique<DefaultObject>();
	floorObject_->Initialize(drawDataManager_->GetDrawData(selectModel.drawDataIndex), selectIndex);
	floorObject_->transform_.position = {4.3f,17.0f,4.8f };
	floorObject_->transform_.scale = { 3.8f,3.0f,3.0f };
	floorObject_->material_.color = { 1.0f,1.0f,1.0f,1.0f };
	floorObject_->material_.isActive = true;
	floorObject_->Update();

	// プレイヤーモデルを取得
	int playerModelID = modelManager_->LoadModel(playerModelName);
	auto playerModel = modelManager_->GetNodeModelData(playerModelID);

	// プレイヤーのテクスチャを取得
	int playerTextureIndex = textureManager_->GetTexture("Player.png");

	// プレイヤーの描画オブジェクト
	playerObject_ = std::make_unique<PlayerUnitObject>();
	playerObject_->Initialize(drawDataManager_->GetDrawData(playerModel.drawDataIndex), playerTextureIndex);
	playerObject_->transform_.position = { 0.0f,-1.4f,0.0f };
	playerObject_->transform_.rotate.y = std::numbers::pi_v<float> / 2;
	currentDir_ = 1.0f;

	/// シーン遷移の処理
	fadeTransition_ = std::make_unique<FadeTransition>();
	fadeTransition_->Initialize(drawDataManager_->GetDrawData(spriteModel.drawDataIndex));

	//=================================================================
	// 
	// タイトルシーンの初期化
	// 
	//=================================================================

	// タイトルロゴ
	int titleLogIndex = textureManager_->GetTexture("Pebble_01.png");

	// タイトルロゴを描画
	titleSprite_ = std::make_unique<SpriteObject>();
	titleSprite_->Initialize(drawDataManager_->GetDrawData(spriteModel.drawDataIndex), { 400.0f * 1.5f,256.0f* 1.5f });
	titleSprite_->transform_.position = { 640.0f,250.0f,0.0f };
	titleSprite_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	titleSprite_->SetTexture(titleLogIndex);
	titleSprite_->Update();

	// タイトルロゴ
	int titlespaceIndex = textureManager_->GetTexture("ClickToStart.png");

	// プレススペース
	spaceSprite_ = std::make_unique<SpriteObject>();
	spaceSprite_->Initialize(drawDataManager_->GetDrawData(spriteModel.drawDataIndex), { 256.0f * 1.3f,64.0f * 1.3f });
	spaceSprite_->transform_.position = { 640.0f,500.0f,0.0f };
	spaceSprite_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	spaceSprite_->SetTexture(titlespaceIndex);
	spaceSprite_->Update();

	// タイトルカメラ
	titleCamera_ = std::make_unique<TitleCamera>();
	titleCamera_->Initialize();

	// 操作UI
	int spIndex = textureManager_->GetTexture("Select_Space.png");
	int leIndex = textureManager_->GetTexture("Select_A.png");
	int riIndex = textureManager_->GetTexture("Select_D.png");
	decisSprite_ = std::make_unique<SpriteObject>();
	decisSprite_->Initialize(drawDataManager_->GetDrawData(spriteModel.drawDataIndex), { 230.0f,83.0f });
	decisSprite_->transform_.position = { 640.0f,640.0f,0.0f };
	decisSprite_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	decisSprite_->SetTexture(spIndex);
	decisSprite_->Update();

	leSprite_ = std::make_unique<SpriteObject>();
	leSprite_->Initialize(drawDataManager_->GetDrawData(spriteModel.drawDataIndex), { 128.0f,128.0f });
	leSprite_->transform_.position = { 96.0f,360.0f,0.0f };
	leSprite_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	leSprite_->SetTexture(leIndex);
	leSprite_->Update();
	isLeftAnimation_ = false;

	riSprite_ = std::make_unique<SpriteObject>();
	riSprite_->Initialize(drawDataManager_->GetDrawData(spriteModel.drawDataIndex), { 128.0f,128.0f });
	riSprite_->transform_.position = { 1184.0f,360.0f,0.0f };
	riSprite_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	riSprite_->SetTexture(riIndex);
	riSprite_->Update();
	isRightAnimation_ = false;

	// おれモデルを取得
	int oreModelID = modelManager_->LoadModel("Ore");
	auto oreModel = modelManager_->GetNodeModelData(oreModelID);

	// おれのテクスチャを取得
	int oreTextureIndex = textureManager_->GetTexture("untitled-0.png");

	// 鉱石モデル
	int mItemModelID = modelManager_->LoadModel("MineralItem");
	auto mItemModel = modelManager_->GetNodeModelData(mItemModelID);

	// 鉱石のテクスチャを取得
	int mItemTextureIndex = textureManager_->GetTexture("Mineral-0.png");

	// 演出
	unitEffectDatas_.resize(10);
	for (size_t i = 0; i < 10; ++i) {
		// ユニット
		unitEffectDatas_[i].unitObject = std::make_unique<DefaultObject>();
		unitEffectDatas_[i].unitObject->Initialize(drawDataManager_->GetDrawData(oreModel.drawDataIndex), oreTextureIndex);
		unitEffectDatas_[i].unitObject->transform_.position = { 0.0f,-1.0f,-5.0f };
		unitEffectDatas_[i].unitObject->transform_.scale = { 1.0f,1.0f,1.0f };
		unitEffectDatas_[i].unitObject->material_.color = { 1.0f,1.0f,1.0f,1.0f };
		unitEffectDatas_[i].unitObject->material_.isActive = true;
		unitEffectDatas_[i].unitObject->Update();
		// アイテム
		unitEffectDatas_[i].itemObject = std::make_unique<DefaultObject>();
		unitEffectDatas_[i].itemObject->Initialize(drawDataManager_->GetDrawData(mItemModel.drawDataIndex), mItemTextureIndex);
		unitEffectDatas_[i].itemObject->transform_.position = { 0.0f,0.0f,-6.0f };
		unitEffectDatas_[i].itemObject->transform_.scale = { 1.0f,1.0f,1.0f };
		unitEffectDatas_[i].itemObject->material_.color = { 1.0f,1.0f,1.0f,1.0f };
		unitEffectDatas_[i].itemObject->material_.isActive = true;
		unitEffectDatas_[i].itemObject->Update();

		unitEffectDatas_[i].isActive_ = false;
		unitEffectDatas_[i].timer_ = 0.0f;
	}

	if (commonData_->isTitle_) {
		isTitle_ = false;
	}

	commonData_->isTitle_ = true;

	spaceSprite_->Update();
	titleSprite_->Update();
	titleCamera_->Update();
	TitleScene();

	// インゲームの処理
	InGameScene();
}

std::unique_ptr<IScene> SelectScene::Update() {

	// Δタイムを取得する
	FpsCount::deltaTime = engine_->GetFPSObserver()->GetDeltatime();

	// 入力処理の更新
	input_->Update();
	commonData_->keyManager->Update();

	// マウスのスクリーン座標を取得する
	POINT cursorPos;
	if (GetCursorPos(&cursorPos)) {
		// スクリーン座標をクライアント座標に変換
		ScreenToClient(gameWindow_->GetWindow()->GetHwnd(), &cursorPos);
		// カーソル位置をワールド座標に変換
		DebugMousePos::screenMousePos = { static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y) };
	}

#ifdef USE_IMGUI
	mousePos_ = DebugMousePos::gameMousePos;
#else
	mousePos_ = DebugMousePos::screenMousePos;
#endif

	debugCamera_->Update();
	camera_ = *static_cast<Camera*>(debugCamera_.get());

	// 演出が終わったので次のシーンに切り替える
	if (fadeTransition_->IsFinished()) {
		isSceneChange_ = true;
	}

	if (fadeTransition_->IsAnimation()) {
		isSceneChange_ = false;
		// シーン遷移
		fadeTransition_->Update();
		isInPlayerAnimation_ = false;
	} else {

		if (isTitle_) {
			// タイトルシーンの更新処理
			TitleScene();
		} else {
			// インゲームの更新処理
			InGameScene();
		}

		// ユニットアニメーションの更新処理
		unitUpdate();
	}

	// シーンを切り替える
	if (isSceneChange_ && fadeTransition_->IsFinished()) {
		return std::make_unique<GameScene>();
	}
	
	return nullptr;
}

void SelectScene::InGameScene() {
	//==================================================
	// ステージの選択処理
	//==================================================

	if (!isInPlayerAnimation_ || !fadeTransition_->IsAnimation()) {
		// キーの状態を取得
		auto key = commonData_->keyManager->GetKeyStates();

		if (key[Key::LeftTri]) {
			if (!selectStageUI_->IsAnimation() && !isPlayerAnimation_) {
				if (selectStageNum_ > 1) {
					selectStageNum_ -= 1;
					selectStageUI_->SetAnimation();
					// プレイヤー関係
					isPlayerAnimation_ = true;
					startPos_ = playerObject_->transform_.position;
					endPos_ = stagePointObjects_[selectStageNum_ - 1]->transform_.position;
					endPos_.y = -1.4f;
					selectDir_ = -1.0f;
					startRotY_ = playerObject_->transform_.rotate.y;
					endRotY_ = -std::numbers::pi_v<float> / 2;
					isLeftAnimation_ = true;

					if (!AudioManager::GetInstance().IsPlay(playerMove_)) {
						AudioManager::GetInstance().Play(playerMove_, 0.5f, false);
					}
				}
			}
		}

		if (key[Key::RightTri]) {
			if (!selectStageUI_->IsAnimation() && !isPlayerAnimation_) {
				if (selectStageNum_ < maxStageNum_) {
					selectStageNum_ += 1;
					selectStageUI_->SetAnimation();
					// プレイヤー関係
					isPlayerAnimation_ = true;
					startPos_ = playerObject_->transform_.position;
					endPos_ = stagePointObjects_[selectStageNum_ - 1]->transform_.position;
					endPos_.y = -1.4f;
					selectDir_ = 1.0f;
					startRotY_ = playerObject_->transform_.rotate.y;
					endRotY_ = std::numbers::pi_v<float> / 2;
					isRightAnimation_ = true;

					if (!AudioManager::GetInstance().IsPlay(playerMove_)) {
						AudioManager::GetInstance().Play(playerMove_, 0.5f, false);
					}
				}
			}
		}

		// マウス位置で選択
		if (leSprite_->transform_.position.x - 80.0f <= mousePos_.x &&
			leSprite_->transform_.position.x + 80.0f >= mousePos_.x) {
			if (leSprite_->transform_.position.y - 80.0f <= mousePos_.y &&
				leSprite_->transform_.position.y + 80.0f >= mousePos_.y) {

				// 左に移動
				if ((Input::GetMouseButtonState()[0] & 0x80) && !(Input::GetPreMouseButtonState()[0] & 0x80)) {
					if (!selectStageUI_->IsAnimation() && !isPlayerAnimation_) {
						if (selectStageNum_ > 1) {
							selectStageNum_ -= 1;
							selectStageUI_->SetAnimation();
							// プレイヤー関係
							isPlayerAnimation_ = true;
							startPos_ = playerObject_->transform_.position;
							endPos_ = stagePointObjects_[selectStageNum_ - 1]->transform_.position;
							endPos_.y = -1.4f;
							selectDir_ = -1.0f;
							startRotY_ = playerObject_->transform_.rotate.y;
							endRotY_ = -std::numbers::pi_v<float> / 2;
							isLeftAnimation_ = true;

							if (!AudioManager::GetInstance().IsPlay(playerMove_)) {
								AudioManager::GetInstance().Play(playerMove_, 0.5f, false);
							}
						}
					}
				}
			}
		}

		if (riSprite_->transform_.position.x - 80.0f <= mousePos_.x &&
			riSprite_->transform_.position.x + 80.0f >= mousePos_.x) {
			if (riSprite_->transform_.position.y - 80.0f <= mousePos_.y &&
				riSprite_->transform_.position.y + 80.0f >= mousePos_.y) {

				// 右に移動
				if ((Input::GetMouseButtonState()[0] & 0x80) && !(Input::GetPreMouseButtonState()[0] & 0x80)) {
					if (!selectStageUI_->IsAnimation() && !isPlayerAnimation_) {
						if (selectStageNum_ < maxStageNum_) {
							selectStageNum_ += 1;
							selectStageUI_->SetAnimation();
							// プレイヤー関係
							isPlayerAnimation_ = true;
							startPos_ = playerObject_->transform_.position;
							endPos_ = stagePointObjects_[selectStageNum_ - 1]->transform_.position;
							endPos_.y = -1.4f;
							selectDir_ = 1.0f;
							startRotY_ = playerObject_->transform_.rotate.y;
							endRotY_ = std::numbers::pi_v<float> / 2;
							isRightAnimation_ = true;

							if (!AudioManager::GetInstance().IsPlay(playerMove_)) {
								AudioManager::GetInstance().Play(playerMove_, 0.5f, false);
							}
						}
					}
				}
			}
		}

		SelectStageNum::num_ = selectStageNum_;

		// 決定
		if (key[Key::Decision]) {
			if (!selectStageUI_->IsAnimation() && !isPlayerAnimation_) {
				// 決定音を鳴らす
				if (!AudioManager::GetInstance().IsPlay(decideSH_)) {
					AudioManager::GetInstance().Play(decideSH_, 0.5f, false);
				}

				if (!AudioManager::GetInstance().IsPlay(playerDesicion_)) {
					AudioManager::GetInstance().Play(playerDesicion_, 0.5f, false);
				}

				commonData_->nextStageIndex = selectStageNum_ - 1;
				commonData_->nextMapIndex = 0;
				commonData_->stageCount = 0;
				commonData_->normaAndScore_.clear();
				commonData_->goldNum = 0;
				commonData_->sumGoldNum_ = 0;
				commonData_->maxGoldNum = 0;
				commonData_->maxOreNum = 0;
				commonData_->killOreNum = 0;
				commonData_->getOreNum = 0;

				isInPlayerAnimation_ = true;
				inPlayerStartRotY_ = playerObject_->transform_.rotate.y;

				//EndlessModeのセット
				if (selectStageNum_ == 3) {
					commonData_->isEndlessMode = true;
					commonData_->goldNum = 0;
				}
			}
		}

		if (decisSprite_->transform_.position.x - 140.0f <= mousePos_.x &&
			decisSprite_->transform_.position.x + 140.0f >= mousePos_.x) {
			if (decisSprite_->transform_.position.y - 50.0f <= mousePos_.y &&
				decisSprite_->transform_.position.y + 50.0f >= mousePos_.y) {

				// 決定
				if ((Input::GetMouseButtonState()[0] & 0x80) && !(Input::GetPreMouseButtonState()[0] & 0x80)) {
					if (!selectStageUI_->IsAnimation() && !isPlayerAnimation_) {
						// 決定音を鳴らす
						if (!AudioManager::GetInstance().IsPlay(decideSH_)) {
							AudioManager::GetInstance().Play(decideSH_, 0.5f, false);
						}

						if (!AudioManager::GetInstance().IsPlay(playerDesicion_)) {
							AudioManager::GetInstance().Play(playerDesicion_, 0.5f, false);
						}

						commonData_->nextStageIndex = selectStageNum_ - 1;
						commonData_->nextMapIndex = 0;
						commonData_->stageCount = 0;
						commonData_->normaAndScore_.clear();
						commonData_->goldNum = 0;
						commonData_->sumGoldNum_ = 0;
						commonData_->maxGoldNum = 0;
						commonData_->maxOreNum = 0;
						commonData_->killOreNum = 0;
						commonData_->getOreNum = 0;

						isInPlayerAnimation_ = true;
						inPlayerStartRotY_ = playerObject_->transform_.rotate.y;

						//EndlessModeのセット
						if (selectStageNum_ == 3) {
							commonData_->isEndlessMode = true;
							commonData_->goldNum = 0;
						}
					}
				}
			}
		}

	}

	//==============================================
	// プレイヤーの更新処理
	//==============================================

	// 移動速度
	Vector3 velocity = { 0.0f,0.0f,0.0f };


	if (isInPlayerAnimation_) {

		inPlayerTimer_ += FpsCount::deltaTime / inPlayerTime_;

		if (inPlayerTimer_ <= 0.3f) {
			float localT = inPlayerTimer_ / 0.3f;
			playerObject_->transform_.rotate.y = lerp(inPlayerStartRotY_, 0.0f, localT, EaseType::EaseInOutCubic);
		}

		// 移動
		playerObject_->transform_.position.z = lerp(-0.5f, 8.0f, inPlayerTimer_, EaseType::EaseInOutCubic);

		if (inPlayerTimer_ <= 0.2f) {
			float localT = inPlayerTimer_ / 0.2f;
			decisSprite_->transform_.scale.x = lerp(230.0f, 128.0f, localT, EaseType::EaseInCubic);
			//decisSprite_->transform_.scale.y = width;
		} else if (inPlayerTimer_ <= 0.4f) {
			float localT = (inPlayerTimer_ - 0.2f) / 0.2f;
			decisSprite_->transform_.scale.x = lerp(128.0f, 230.0f, localT, EaseType::EaseOutCubic);
			//decisSprite_->transform_.scale.y = width;
		} 

		if (inPlayerTimer_ >= 1.0f) {
			fadeTransition_->SetFade(FadeTransition::Phase::Out);
		}

		decisSprite_->Update();
	} else {
		if (isPlayerAnimation_) {
			timer_ += FpsCount::deltaTime / moveTime_;

			Vector3 prePos = playerObject_->transform_.position;

			// 回転
			if (currentDir_ != selectDir_) {
				if (timer_ <= 0.3f) {
					float localT = timer_ / 0.3f;
					playerObject_->transform_.rotate.y = lerp(startRotY_, endRotY_, localT, EaseType::EaseInOutCubic);
				} else {
					playerObject_->transform_.rotate.y = endRotY_;
				}
			}

			// UIのアニメーションを追加
			if (isLeftAnimation_) {
				if (timer_ <= 0.5f) {
					float localT = timer_ / 0.5f;
					float width = lerp(128.0f,64.0f,localT,EaseType::EaseInCubic);
					leSprite_->transform_.scale.x = width;
					leSprite_->transform_.scale.y = width;
				} else {
					float localT = (timer_ - 0.5f) / 0.5f;
					float width = lerp(64.0f, 128.0f, localT, EaseType::EaseOutCubic);
					leSprite_->transform_.scale.x = width;
					leSprite_->transform_.scale.y = width;
				}
			}

			if(isRightAnimation_) {
				if (timer_ <= 0.5f) {
					float localT = timer_ / 0.5f;
					float width = lerp(128.0f, 64.0f, localT, EaseType::EaseInCubic);
					riSprite_->transform_.scale.x = width;
					riSprite_->transform_.scale.y = width;
				} else {
					float localT = (timer_ - 0.5f) / 0.5f;
					float width = lerp(64.0f, 128.0f, localT, EaseType::EaseOutCubic);
					riSprite_->transform_.scale.x = width;
					riSprite_->transform_.scale.y = width;
				}
			}

			// 移動
			playerObject_->transform_.position = lerp(startPos_, endPos_, timer_, EaseType::EaseInOutCubic);

			velocity = playerObject_->transform_.position - prePos;

			if (timer_ >= 1.0f) {
				isLeftAnimation_ = false;
				isRightAnimation_ = false;
				currentDir_ = selectDir_;
				timer_ = 0.0f;
				isPlayerAnimation_ = false;
				playerObject_->transform_.position = endPos_;

				float width = 128.0f;
				leSprite_->transform_.scale.x = width;
				leSprite_->transform_.scale.y = width;
				riSprite_->transform_.scale.x = width;
				riSprite_->transform_.scale.y = width;
			}
		}
	}

	// プレイヤーの更新処理
	playerObject_->Update();

	leSprite_->Update();
	riSprite_->Update();
	floorObject_->Update();

	//=====================================================
	// カメラの更新処理
	//=====================================================

	// カメラの追跡位置を更新
	selectCamera_->SetTargetPos(playerObject_->transform_.position, velocity);
	selectCamera_->Update();

	//==================================================
	// UIの更新処理
	//==================================================

	// ステージ選択の更新処理
	selectStageUI_->Update();
}

void SelectScene::TitleScene() {

	// キーの状態を取得
	auto key = commonData_->keyManager->GetKeyStates();

	if (key[Key::Decision] || ((Input::GetMouseButtonState()[0] & 0x80) && !(Input::GetPreMouseButtonState()[0] & 0x80))) {
		isStartTitle_ = true;

		// 決定音を鳴らす
		if (!AudioManager::GetInstance().IsPlay(decideSH_)) {
			AudioManager::GetInstance().Play(decideSH_, 0.5f, false);
		}
	}

	// カメラの更新処理
	//titleCamera_->Update();

	if (!isStartTitle_) { return; }

	titleTimer_ += FpsCount::deltaTime / titleTime_;

	// 移動
	if (titleTimer_ <= 1.0f) {
		titleSprite_->transform_.position.y = lerp(250.0f, -360.0f, titleTimer_, EaseType::EaseInOutCubic);
	}

	if (titleTimer_ <= 0.5f) {
		float localT = titleTimer_ / 0.5f;
		spaceSprite_->color_.w = lerp(1.0f, 0.0f, localT, EaseType::EaseInOutCubic);
	}
	spaceSprite_->Update();

	// 更新処理
	titleSprite_->Update();

	// カメラの更新処理
	titleCamera_->Update();

	if (titleTimer_ >= 1.0f) {

		if (!titleCamera_->IsAnimation()) {
			isTitle_ = false;
		}
	}
}

void SelectScene::unitUpdate() {

	//=================================================
	// 発射処理
	//=================================================

	coolTimer_ += FpsCount::deltaTime / coolTime_;

	if (coolTimer_ >= 1.0f) {
		coolTimer_ = 0.0f;

		coolTime_ = RandomGenerator::Get(0.5f, 1.2f);

		for (auto& data : unitEffectDatas_) {
			if (!data.isActive_) {
				data.isActive_ = true;
				data.timer_ = 0.0f;
				break;
			}
		}
	}

	//==================================================
	// 移動処理
	//==================================================

	float maxJumpHeight_ = 0.8f;
	float maxWidth_ = 1.2f;
	float minWidth_ = 0.8f;
	float maxHeight_ = 1.2f;
	float minHeight_ = 0.8f;

	for (auto& data : unitEffectDatas_) {
		if (data.isActive_) {

			data.timer_ += FpsCount::deltaTime / 5.0f;

			// 移動
			data.unitObject->transform_.position.x = lerp(-15.0f, 20.0f, data.timer_, EaseType::Linear);
		
			// アニメーション
			data.animationTimer_ += FpsCount::deltaTime / 0.6f;
			if (data.animationTimer_ <= 0.5f) {
				float localT = data.animationTimer_ / 0.5f;
				data.unitObject->transform_.position.y = lerp(-1.0f, maxJumpHeight_, localT, EaseType::EaseInOutCubic);

				// スケール
				float width = lerp(maxWidth_, minWidth_, localT, EaseType::EaseInOutCubic);
				data.unitObject->transform_.scale.x = width;
				data.unitObject->transform_.scale.z = width;
				data.unitObject->transform_.scale.y = lerp(minHeight_, maxHeight_, localT, EaseType::EaseInOutCubic);
			} else {
				float localT = (data.animationTimer_ - 0.5f) / 0.5f;
				data.unitObject->transform_.position.y = lerp(maxJumpHeight_, -1.0f, localT, EaseType::EaseInCubic);
				// スケール
				float width = lerp(minWidth_, maxWidth_, localT, EaseType::EaseInOutCubic);
				data.unitObject->transform_.scale.x = width;
				data.unitObject->transform_.scale.z = width;
				data.unitObject->transform_.scale.y = lerp(maxHeight_, minHeight_, localT, EaseType::EaseInOutCubic);
			}

			if (data.animationTimer_ >= 1.0f) {
				data.animationTimer_ = 0.0f;
			}

			// 鉱石を移動
			data.itemObject->transform_.position = data.unitObject->transform_.position + Vector3(0, 0.7f, 0);

			if (data.timer_ >= 1.0f) {
				data.isActive_ = false;
			}

			// 更新処理
			data.unitObject->Update();
			data.itemObject->Update();
		}
	}
}

void SelectScene::Draw() {
	display_->PreDraw(gameWindow_->GetCommandObject(), true);

	Matrix4x4 vpMatrix = selectCamera_->GetVpMatrix();
	if (isTitle_) {
		vpMatrix = titleCamera_->GetVpMatrix();
	}

	// ステージを描画
	for (auto& point : stagePointObjects_) {
		point->Draw(gameWindow_->GetWindow(), vpMatrix);
	}

	// 足場を描画
	floorObject_->Draw(gameWindow_->GetWindow(), vpMatrix);

	// 洞窟を描画
	for (int i = 0; i < caveObjects_.size(); ++i) {
		caveObjects_[i]->Draw(gameWindow_->GetWindow(), vpMatrix);
	}

	// ユニットを描画
	for (auto& data : unitEffectDatas_) {
		if (data.isActive_) {
			data.unitObject->Draw(gameWindow_->GetWindow(), vpMatrix);
			data.itemObject->Draw(gameWindow_->GetWindow(), vpMatrix);
		}
	}

	// プレイヤーの描画
	playerObject_->Draw(gameWindow_->GetWindow(),vpMatrix);

	// UIの描画(ポストエフェクトなどを掛ける場合、掛からないように描画場所を変更する)

	vpMatrix = uiCamera_->GetVPMatrix();

	if (isTitle_) {
		// タイトルロゴの描画
		titleSprite_->Draw(gameWindow_->GetWindow(), vpMatrix);

		// 操作UI
		spaceSprite_->Draw(gameWindow_->GetWindow(), vpMatrix);
	} else {
		// ステージの選択
		selectStageUI_->Draw(gameWindow_->GetWindow(), vpMatrix);

		// 操作方法
		decisSprite_->Draw(gameWindow_->GetWindow(), vpMatrix);
		leSprite_->Draw(gameWindow_->GetWindow(), vpMatrix);
		riSprite_->Draw(gameWindow_->GetWindow(), vpMatrix);
	}

	// シーン遷移の描画
	fadeTransition_->Draw(gameWindow_->GetWindow(), vpMatrix);

	display_->PostDraw(gameWindow_->GetCommandObject());

	//PostEffectとか9i[

	postEffectConfig_.output = gameWindow_->GetDualDisplay();
	postEffect_->Draw(postEffectConfig_);

	gameWindow_->PreDraw(false);

	//ImGui
	gameWindow_->DrawDisplayWithImGui();
	paramManager_->Draw();

	selectCamera_->DrawImGui();

	//titleCamera_->DebugDraw();

#ifdef USE_IMGUI
	ImGui::Begin("Titel");
	ImGui::DragFloat3("pos", &floorObject_->transform_.position.x, 0.1f);
	ImGui::DragFloat3("size", &floorObject_->transform_.scale.x, 0.1f);
	ImGui::End();
#endif
	engine_->ImGuiDraw();
}
