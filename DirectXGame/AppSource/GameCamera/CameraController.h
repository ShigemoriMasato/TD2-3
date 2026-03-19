#pragma once
#include"Camera/Camera.h"
#include"Input/Input.h"

#include"GameCommon/DefaultObject.h"
#include"MarkResult.h"

class CameraController : public Camera {
public:

	void Initialize(Input* input, DrawData drawData, int texture);

	void Update(bool isDebug = false);

	void DebugDraw();

	/// <summary>
	/// マウスのスクリーン座標を3D空間の座標に変換
	/// </summary>
	/// <param name="screenPos">マウスのスクリーン座標</param>
	/// <param name="screenWidth">スクリーンの幅</param>
	/// <param name="screenHeight">スクリーンの高さ</param>
	/// <returns>3D空間上の座標</returns>
	Vector3 ScreenToWorld(const Vector2& screenPos, float screenWidth = 1280.0f, float screenHeight = 720.0f) const;

	// アニメーションを描画
	void DrawAnimation(Window* window, const Matrix4x4& vpMatrix);

public:

	Matrix4x4 GetVpMatrix() const { return vpMatrix_; }

	Vector3 GetWorldPos() { return worldPos_; }

	// カメラのワールド座標位置を取得
	Vector3 GetCameraWorldPos() {
		// ワールド座標を入れる変数
		Vector3 worldPos;
		// ワールド行列の平行移動成分を取得
		worldPos.x = worldMatrix_.m[3][0];
		worldPos.y = worldMatrix_.m[3][1];
		worldPos.z = worldMatrix_.m[3][2];
		return worldPos;
	}

	float GetRange() const { return backDist_ * -1.0f; }

	// マップの最大サイズを取得
	void SetMapMaxSize(const Vector2& size) {
		mapMaxSize_ = size;
	}

	// アニメーションの開始
	void StartAnimation() {
		if (!isAnimation_) {
			isAnimation_ = true;
			clickObject_->transform_.position = worldPos_ + Vector3(0.0f,0.2f,0.0f);
		}
	}

	void SetTargetPos(const Vector3 pos) {
		targetPos_ = pos;
	}

	/// <summary>
	/// 3D空間の座標をスクリーン座標に変換する
	/// </summary>
	/// <param name="worldPos">変換したいワールド座標</param>
	/// <returns>x,y:スクリーン座標, z:カメラ前方への距離(w)</returns>
	Vector3 WorldToScreen(const Vector3& worldPos) const;

	MarkerResult GetMarkerInfo(const Vector3& targetWorldPos, float margin, float screenWidth = 1280.0f, float screenHeight = 720.0f);

private:

	void MakeMatrix() override;


	Vector3 backDir_ = { 0.0f, -0.94f, 0.4f };
	float backDist_ = -40.0f;
	float targetBackDist_ = -40.0f;
	float initBackDist_ = 0.0f;
	const float normalEaseSpeed_ = 10.0f;
	float easeSpeed_ = 10.0f;

	Input* input_ = nullptr;
	//クリックした瞬間のマウス座標
	Vector3 clickedCameraPos_ = {};
	Vector2 mousePos_ = {};
	bool preClicked_ = false;
	float deadZone_ = 10.0f;

	float dirMax_ = -10.0f;
	float dirMin_ = -100.0f;

	// なぜかカメラのワールド行列がないので追加
	Matrix4x4 worldMatrix_;

	// 移動速度
	float moveSpeed_ = 30.0f;

	// マップの最大サイズ
	Vector2 mapMaxSize_ = { 100.0f,100.0f };

	// ワールド座標の位置
	Vector3 worldPos_ = {};

	// クリックアニメーション
	std::unique_ptr<DefaultObject> clickObject_;

	bool isAnimation_ = false;
	bool isFollow_ = true;

	float timer_ = 0.0f;
	float maxTime_ = 0.3f;

private://追従カメラ
	Vector3 targetPos_ = {};
	float followSpeed_ = 5.0f;

	bool initialize_ = false;
	int initializeFrame_ = 0;

	float initializeSpeed_ = 3.0f;

private:

	// クリックアニメーション
	void ClickAnimation();
};