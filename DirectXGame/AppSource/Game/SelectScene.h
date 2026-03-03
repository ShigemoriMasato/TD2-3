#pragma once
#include <Scene/IScene.h>
#include <Camera/DebugCamera.h>
#include <Common/DebugParam/ParamManager.h>
#include <Render/PostEffect.h>
#include <Collision/ColliderManager.h>

#include<vector>
#include<array>

#include"Unit/Player/PlayerUnitObject.h"
#include"UI/SelectStageUI.h"
#include"GameCommon/DefaultObject.h"
#include"GameCamera/SelectCamera.h"
#include"UI/FadeTransition.h"
#include"UI/Object/SpriteObject.h"
#include"GameCamera/TitleCamera.h"

class SelectScene : public IScene {
public:

	~SelectScene();

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private://エンジン的に必要な奴ら

	DualDisplay* display_ = nullptr;
	GameWindow* gameWindow_ = nullptr;

private:

	std::unique_ptr<ParamManager> paramManager_ = nullptr;

	std::unique_ptr<PostEffect> postEffect_ = nullptr;
	PostEffectConfig postEffectConfig_{};
	Blur blur_{};

	// 遷移の処理
	std::unique_ptr<FadeTransition> fadeTransition_;

	// シーン切り替え
	bool isSceneChange_ = false;

	// BGMハンドル
	uint32_t selectSH_ = 0;

	// マウス座標
	Vector2 mousePos_ = {};

	//=================================
	// カメラ
	//=================================

	std::unique_ptr<DebugCamera> debugCamera_ = nullptr;
	Camera camera_;

	// ui描画用のカメラ
	std::unique_ptr<Camera> uiCamera_;

	// プレイヤーに追従するカメラ
	std::unique_ptr<SelectCamera> selectCamera_;

	//==============================================
	// プレイヤー
	//==============================================

	// プレイヤーのオブジェクトデータ
	std::unique_ptr<PlayerUnitObject> playerObject_;

	bool isPlayerAnimation_ = false;

	// 洞窟に入る演出
	bool isInPlayerAnimation_ = false;
	float inPlayerTimer_ = 0.0f;
	float inPlayerTime_ = 1.0f;
	float inPlayerStartRotY_ = 0.0f;

	Vector3 startPos_ = {};
	Vector3 endPos_ = {};

	float timer_ = 0.0f;
	float moveTime_ = 1.0f;

	float currentDir_ = 1.0f;
	float selectDir_ = 1.0f;

	float startRotY_ = 0.0f;
	float endRotY_ = 0.0f;

	//====================================================
	// ステージ
	//====================================================

	std::array<std::unique_ptr<DefaultObject>,3> stagePointObjects_;

	// 洞窟描画用
	std::array<std::unique_ptr<DefaultObject>, 3> caveObjects_;
	
	// 足場オブジェクト
	std::unique_ptr<DefaultObject> floorObject_;

	//==============================================
	// UI
	//==============================================

	// 選択しているステージ番号
	int selectStageNum_ = 1;
	int maxStageNum_ = 3;

	// ステージ選択のロゴ
	std::unique_ptr<SelectStageUI> selectStageUI_;

	// スペース
	std::unique_ptr<SpriteObject> decisSprite_;

	std::unique_ptr<SpriteObject> leSprite_;
	bool isLeftAnimation_ = false;

	std::unique_ptr<SpriteObject> riSprite_;
	bool isRightAnimation_ = false;

	//===========================================================
	// 
	// タイトルシーン
	// 
	//===========================================================

	// タイトルロゴ
	std::unique_ptr<SpriteObject> titleSprite_;

	// プレススペース
	std::unique_ptr<SpriteObject> spaceSprite_;

	float titleTimer_ = 0.0f;
	float titleTime_ = 1.0f;
	bool isTitle_ = true;

	bool isStartTitle_ = false;

	std::unique_ptr<TitleCamera> titleCamera_;

	//======================================================
	//
	// 音声
	//
	//======================================================

	// 決定音
	uint32_t decideSH_ = 0;

	// 決定
	uint32_t playerDesicion_ = 0;
	// 移動
	uint32_t playerMove_ = 0;

	//========================================================
	// ユニットの演出
	//========================================================

	// 演出用のユニットデータ
	struct UnitEffectData {
		std::unique_ptr<DefaultObject> unitObject;
		std::unique_ptr<DefaultObject> itemObject;
		float timer_ = 0.0f;

		float animationTimer_ = 0.0f;

		bool isActive_ = false;
	};

	// ユニットデータ
	std::vector<UnitEffectData> unitEffectDatas_;

	float coolTimer_ = 0.0f;
	float coolTime_ = 1.0f;

private:

	// ゲームの実行処理
	void InGameScene();

	// タイトルシーン
	void TitleScene();

	// ユニットの更新処理
	void unitUpdate();
};
