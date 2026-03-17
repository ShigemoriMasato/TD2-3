#include"CameraController.h"
#include<numbers>
#include<Windows.h>
#include"DebugMousePos.h"
#include"FpsCount.h"
#include<algorithm>
#include"Utility/Easing.h"
#include <Common/DebugParam/GameParamEditor.h>

#ifdef USE_IMGUI
#include <imgui/imgui.h>
#endif

void CameraController::Initialize(Input* input, DrawData drawData, int texture) {

	// 入力処理を取得
	input_ = input;

	// カメラを初期化
	this->SetProjectionMatrix(PerspectiveFovDesc{});
	position_ = { 0.0f,8.0f,0.0f };
	rotation_ = { -1.17f,0.0f,0.0f };
	MakeMatrix();

	// クリック位置を描画
	clickObject_ = std::make_unique<DefaultObject>();
	clickObject_->Initialize(drawData, texture);

	vpMatrix_ = Matrix4x4::Identity();
	worldMatrix_ = Matrix4x4::Identity();
	transformMatrix_ = Matrix4x4::Identity();
  
	auto edit = GameParamEditor::GetInstance();
	edit->CreateGroup("Camera", "GameScene");
	edit->AddItem("Camera", "InitBackDist", initBackDist_, 1);
	edit->AddItem("Camera", "FollowSpeed", followSpeed_, 2);
	edit->AddItem("Camera", "Min", dirMin_, 3);
	edit->AddItem("Camera", "Max", dirMax_, 4);
	initBackDist_ = edit->GetValue<float>("Camera", "InitBackDist");

	backDist_ = initBackDist_;
	targetBackDist_ = initBackDist_;

	initialize_ = false;
	initializeFrame_ = 0;
}

void CameraController::Update(bool isDebug) {

	easeSpeed_ = normalEaseSpeed_;

	if (initializeFrame_ == 0) {
		position_ = targetPos_;
	}

	//しばらく0で固定する
	if (initializeFrame_ < 120) {
		initializeFrame_++;
		backDist_ = 0.0f;
	}

	if (initializeFrame_ < 150) {
		easeSpeed_ = initializeSpeed_;
	}

	if (isDebug) {
		easeSpeed_ = normalEaseSpeed_ * 2.0f;
	}

	auto edit = GameParamEditor::GetInstance();
	initBackDist_ = edit->GetValue<float>("Camera", "InitBackDist");
	followSpeed_ = edit->GetValue<float>("Camera", "FollowSpeed");
	dirMin_ = edit->GetValue<float>("Camera", "Min");
	dirMax_ = edit->GetValue<float>("Camera", "Max");

	if (isFollow_) {
		
		float dist = (targetPos_ - position_).Length();

		if (dist > 1.0f * FpsCount::deltaTime) {
			Vector3 dir = (targetPos_ - position_).Normalize();
			position_ += dir * dist * followSpeed_ * FpsCount::deltaTime;
		} else {
			position_ = targetPos_;
		}

		if (input_->GetMouseButtonState()[1]) {
			isFollow_ = false;
		}

	} else {
		// 0:左, 1:右, 2:中
		BYTE* mouseKey = input_->GetMouseButtonState();

		// 左クリックを押している状態
		if (mouseKey && (mouseKey[1]) && DebugMousePos::isGrabbed) {
			if (!preClicked_) {
				mousePos_ = {};
				clickedCameraPos_ = position_;
				preClicked_ = true;
			}
			// マウスの移動量を座標に追加
			mousePos_ += input_->GetMouseMove();
			float length = mousePos_.Length();
			if (length > deadZone_) {
				Vector2 move = (length - deadZone_) * mousePos_.Normalize() * 0.05f;
				position_ = clickedCameraPos_ + Vector3(-move.x, 0, move.y);
			}
		} else {
			preClicked_ = false;
		}

		if (input_->GetKeyState(DIK_LSHIFT)) {
			isFollow_ = true;
		}
	}


	if (input_->GetKeyState(DIK_LSHIFT)) {
		targetBackDist_ = initBackDist_;
	}

	if (DebugMousePos::isHovered) {
		targetBackDist_ += input_->GetMouseWheel() * 0.02f;
	}

	float diff = backDist_ - targetBackDist_;
	if (std::abs(diff) < 0.5f * FpsCount::deltaTime) {
		backDist_ = targetBackDist_;
	} else {
		float speed = -diff * easeSpeed_;
		backDist_ += speed * FpsCount::deltaTime;
	}

	if (dirMin_ > dirMax_) {
		std::swap(dirMin_, dirMax_);
	}

	// カメラの移動範囲を制限する
	position_.x = std::clamp(position_.x,0.0f, mapMaxSize_.x * 1.5f);
	position_.y = 0.0f;
	position_.z = std::clamp(position_.z, -3.0f, mapMaxSize_.y * 0.8f);

	// カメラの喀出を制限(カメラの見える範囲は固定になると思うのでいらないかも)
	backDist_ = std::clamp(backDist_, dirMin_, dirMax_);
	targetBackDist_ = std::clamp(targetBackDist_, dirMin_, dirMax_);

	// カメラの更新処理
	MakeMatrix();

	// ワールド座標の位置を取得する
#ifdef USE_IMGUI
	worldPos_ = ScreenToWorld(DebugMousePos::gameMousePos, 1280.0f, 720.0f);
#else
	worldPos_ = ScreenToWorld(DebugMousePos::screenMousePos, 1280.0f, 720.0f);
#endif

	// クリック時のアニメーションを描画
	if (isAnimation_) {
		ClickAnimation();
	}

}

void CameraController::DebugDraw() {
	// デバック描画
	DrawImGui();
#ifdef USE_IMGUI
	ImGui::Begin("DebugMouse");
	ImGui::Text("GameMousePos x : %.2f, y : %.2f", DebugMousePos::gameMousePos.x, DebugMousePos::gameMousePos.y);
	ImGui::Text("WindowMousePos x : %.2f, y : %.2f", DebugMousePos::windowPos.x, DebugMousePos::windowPos.y);
	ImGui::Text("ScreenMousePos x : %.2f, y : %.2f", DebugMousePos::screenMousePos.x, DebugMousePos::screenMousePos.y);
	ImGui::Text("WorldMousePos x : %.2f, y : %.2f, z : %.2f", worldPos_.x, worldPos_.y, worldPos_.z);
	ImGui::Text("BackDist : %.2f", backDist_);
	ImGui::DragFloat3("BackDir", &backDir_.x, 0.01f);
	backDir_ = backDir_.Normalize();

	ImGui::Text("InitDist: %f", initBackDist_);
	ImGui::Text("TargetDist: %f, CurrentDist: %f", targetBackDist_, backDist_);
	ImGui::End();
#endif
}

Vector3 CameraController::ScreenToWorld(const Vector2& screenPos, float screenWidth, float screenHeight) const {
	// 1. スクリーン座標をNDC（正規化デバイス座標）に変換
	// NDCは[-1, 1]の範囲（左下が(-1,-1)、右上が(1,1)）
	Vector2 ndc;
	ndc.x = (screenPos.x / screenWidth) * 2.0f - 1.0f;
	ndc.y = -((screenPos.y / screenHeight) * 2.0f - 1.0f); // Y軸を反転

	// 2. ビュー行列とプロジェクション行列の逆行列を取得
	Matrix4x4 viewMatrix = transformMatrix_;
	
	Matrix4x4 invView = Matrix::InverseMatrix(viewMatrix);
	Matrix4x4 invProjection = Matrix::InverseMatrix(projectionMatrix_);

	// 3. NDCからビュー空間への変換（Near平面とFar平面）
	Vector4 nearPoint = Vector4(ndc.x, ndc.y, 0.0f, 1.0f); // Near平面
	Vector4 farPoint = Vector4(ndc.x, ndc.y, 1.0f, 1.0f);  // Far平面

	// プロジェクション逆変換
	Vector4 nearView = {
		nearPoint.x * invProjection.m[0][0] + nearPoint.y * invProjection.m[1][0] + nearPoint.z * invProjection.m[2][0] + nearPoint.w * invProjection.m[3][0],
		nearPoint.x * invProjection.m[0][1] + nearPoint.y * invProjection.m[1][1] + nearPoint.z * invProjection.m[2][1] + nearPoint.w * invProjection.m[3][1],
		nearPoint.x * invProjection.m[0][2] + nearPoint.y * invProjection.m[1][2] + nearPoint.z * invProjection.m[2][2] + nearPoint.w * invProjection.m[3][2],
		nearPoint.x * invProjection.m[0][3] + nearPoint.y * invProjection.m[1][3] + nearPoint.z * invProjection.m[2][3] + nearPoint.w * invProjection.m[3][3]
	};
	nearView = nearView / nearView.w; // 同次座標を正規化

	Vector4 farView = {
		farPoint.x * invProjection.m[0][0] + farPoint.y * invProjection.m[1][0] + farPoint.z * invProjection.m[2][0] + farPoint.w * invProjection.m[3][0],
		farPoint.x * invProjection.m[0][1] + farPoint.y * invProjection.m[1][1] + farPoint.z * invProjection.m[2][1] + farPoint.w * invProjection.m[3][1],
		farPoint.x * invProjection.m[0][2] + farPoint.y * invProjection.m[1][2] + farPoint.z * invProjection.m[2][2] + farPoint.w * invProjection.m[3][2],
		farPoint.x * invProjection.m[0][3] + farPoint.y * invProjection.m[1][3] + farPoint.z * invProjection.m[2][3] + farPoint.w * invProjection.m[3][3]
	};
	farView = farView / farView.w; // 同次座標を正規化

	// 4. ビュー空間からワールド空間への変換
	Vector3 nearWorld = {
		nearView.x * invView.m[0][0] + nearView.y * invView.m[1][0] + nearView.z * invView.m[2][0] + invView.m[3][0],
		nearView.x * invView.m[0][1] + nearView.y * invView.m[1][1] + nearView.z * invView.m[2][1] + invView.m[3][1],
		nearView.x * invView.m[0][2] + nearView.y * invView.m[1][2] + nearView.z * invView.m[2][2] + invView.m[3][2]
	};

	Vector3 farWorld = {
		farView.x * invView.m[0][0] + farView.y * invView.m[1][0] + farView.z * invView.m[2][0] + invView.m[3][0],
		farView.x * invView.m[0][1] + farView.y * invView.m[1][1] + farView.z * invView.m[2][1] + invView.m[3][1],
		farView.x * invView.m[0][2] + farView.y * invView.m[1][2] + farView.z * invView.m[2][2] + invView.m[3][2]
	};

	// 5. レイの方向を計算
	Vector3 rayDirection = (farWorld - nearWorld).Normalize();

	// 6. レイとY=0平面の交点を求める
	// 平面の方程式: Y = 0
	// レイの方程式: P = nearWorld + t * rayDirection
	// 交点: nearWorld.y + t * rayDirection.y = 0
	// t = -nearWorld.y / rayDirection.y
	
	if (std::abs(rayDirection.y) < 0.0001f) {
		// レイが平面と平行な場合
		return Vector3(0.0f, 0.0f, 0.0f);
	}

	float t = -nearWorld.y / rayDirection.y;
	Vector3 intersectionPoint = nearWorld + rayDirection * t;

	return intersectionPoint;
}

void CameraController::MakeMatrix() {
	Vector3 offset = backDir_.Normalize() * backDist_;
	transformMatrix_ = Matrix::MakeTranslationMatrix(-position_ - offset) * Matrix::MakeRotationMatrix(rotation_);
	worldMatrix_ = Matrix::MakeAffineMatrix(scale_,rotation_,position_);
	//vpMatrix_ = transformMatrix_ * projectionMatrix_;
	vpMatrix_ = Matrix::MakeTranslationMatrix(-position_ - offset) * Matrix::MakeRotationMatrix(rotation_) * projectionMatrix_;
}

void CameraController::DrawAnimation(Window* window, const Matrix4x4& vpMatrix) {
	// クリックアニメーションを描画
	clickObject_->Draw(window, vpMatrix);
}

void CameraController::ClickAnimation() {

	timer_ += FpsCount::deltaTime / maxTime_;

	float width = lerp(1.0f, 2.0f, timer_, EaseType::EaseInOutCubic);
	clickObject_->transform_.scale.x = width;
	clickObject_->transform_.scale.z = width;

	clickObject_->material_.color.w = lerp(1.0f, 0.0f, timer_, EaseType::Linear);

	if (timer_ >= 1.0f) {
		timer_ = 0.0f;
		isAnimation_ = false;
	}

	// 更新処理
	clickObject_->Update();
}

Vector3 CameraController::WorldToScreen(const Vector3& worldPos) const {
	// VP行列を取得
	Matrix4x4 mat = vpMatrix_;

	// ワールド座標をクリップ空間座標に変換
	float w = worldPos.x * mat.m[0][3] + worldPos.y * mat.m[1][3] + worldPos.z * mat.m[2][3] + mat.m[3][3];

	// カメラの真横や後ろにある場合
	// スクリーン座標としては無効だが、判定用にwをそのまま返すか、例外的な値を返す
	if (w <= 0.0f) {
		return Vector3(0.0f, 0.0f, w);
	}

	float x = worldPos.x * mat.m[0][0] + worldPos.y * mat.m[1][0] + worldPos.z * mat.m[2][0] + mat.m[3][0];
	float y = worldPos.x * mat.m[0][1] + worldPos.y * mat.m[1][1] + worldPos.z * mat.m[2][1] + mat.m[3][1];

	// 透視投影除算
	float ndcX = x / w;
	float ndcY = y / w;

	// スクリーン座標系に変換
	Vector3 screenPos;
	screenPos.x = (ndcX + 1.0f) * 0.5f * 1280.0f;
	screenPos.y = (1.0f - ndcY) * 0.5f * 720.0f;
	screenPos.z = w;

	return screenPos;
}

MarkerResult CameraController::GetMarkerInfo(const Vector3& targetWorldPos, float margin, float screenWidth, float screenHeight) {
	MarkerResult result;
	Vector2 center = { screenWidth * 0.5f, screenHeight * 0.5f };

	// ワールド座標をスクリーン座標に変換
	Vector3 screenPos3D = WorldToScreen(targetWorldPos);
	Vector2 targetScreenPos = { screenPos3D.x, screenPos3D.y };
	float w = screenPos3D.z;

	// カメラの後ろにいる場合の補正
	if (w < 0.0f) {
		targetScreenPos = center - (targetScreenPos - center);
	}

	// 画面内判定
	if (w > 0.0f &&
		targetScreenPos.x >= margin && targetScreenPos.x <= (screenWidth - margin) &&
		targetScreenPos.y >= margin && targetScreenPos.y <= (screenHeight - margin)) {
		// 画面内にいる
		result.isVisible = true;
		result.position = targetScreenPos;
		result.rotation = 0.0f;
		return result;
	}

	// 画面外にいる
	result.isVisible = false;

	// 中心からターゲットへの方向ベクトルを計算
	Vector2 dir = targetScreenPos - center;

	// 回転角度を計算
	result.rotation = std::atan2(dir.y, dir.x);

	// 画面半分のサイズ。空白を考慮
	float halfW = (screenWidth * 0.5f) - margin;
	float halfH = (screenHeight * 0.5f) - margin;

	// 比率を計算して、枠に収まるようにスケーリング
	float slope = (dir.x != 0.0f) ? (dir.y / dir.x) : 0.0f;

	if (dir.x == 0.0f && dir.y == 0.0f) {
		// 重なっている場合は中心
		result.position = center;
	} else {
		// 矩形の境界との交点計算
		// X軸方向の壁までの距離比率
		float factor = 1.0f;

		// とりあえずX軸基準で壁にぶつかると仮定
		if (std::abs(dir.x) > 0.001f) {
			float xLimit = (dir.x > 0) ? halfW : -halfW;
			factor = xLimit / dir.x;
		} else {
			factor = 10000.0f;
		}

		// Y軸方向の壁チェック
		if (std::abs(dir.y * factor) > halfH) {
			float yLimit = (dir.y > 0) ? halfH : -halfH;
			factor = yLimit / dir.y;
		}

		// 中心座標に、スケーリングしたベクトルを加算
		result.position = center + (dir * factor);
	}

	return result;
}