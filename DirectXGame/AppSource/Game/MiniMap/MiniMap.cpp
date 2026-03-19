#include "MiniMap.h"
#include <numbers>
#include <GameCamera/DebugMousePos.h>
#include <Input/Input.h>
#include <Common/DebugParam/GameParamEditor.h>

namespace {
	void Log(Matrix4x4& mat, Logger logger) {
		for (int i = 0; i < 4; ++i) {
			float* tmp = mat.m[i];
			logger->info("{} : {}, {}, {}, {}", i, tmp[0], tmp[1], tmp[2], tmp[3]);
		}
	}
}

void MiniMap::Initialize(int mapWidth, int mapHeight, DualDisplay* disp, const DrawData& plane, const DrawData& visionField, TextureManager* textureManager) {
	auto logger = getLogger("Minimap");

	// カメラの初期化
	camera_ = std::make_unique<Camera>();
	camera_->SetProjectionMatrix(PerspectiveFovDesc());

	transform_.rotate.x = std::numbers::pi_v<float> / 2.0f;
	distRatio_ = 0.75f;

	dist = std::max(mapWidth, mapHeight) * 0.5f / std::tan(0.45f / 2.0f);
	transform_.position = { mapWidth / 2.0f - 0.5f, 0.0f, mapHeight / 2.0f - 0.5f };
	Vector3 cameraPos = transform_.position + Vector3{ 0.0f, dist * distRatio_, 0.0f };
	camera_->SetTransform(Matrix::MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, transform_.rotate, { cameraPos }).Inverse());
	camera_->MakeMatrix();

	logger->info("Positoin: {}, {}, {}", cameraPos.x, cameraPos.y, cameraPos.z);
	logger->info("Scale: {}, {}, {}", transform_.scale.x, transform_.scale.y, transform_.scale.z);

	Matrix4x4 transformMatrix = camera_->GetTransformMatrix();
	logger->info("TransformMatrix");
	Log(transformMatrix, logger);

	Matrix4x4 vpMat = camera_->GetVPMatrix();
	Log(vpMat, logger);

	display_ = disp;

	miniMapRender_ = std::make_unique<RenderObject>();
	miniMapRender_->Initialize();
	miniMapRender_->SetDrawData(plane);
	miniMapRender_->CreateCBV(sizeof(Matrix4x4), ShaderType::VERTEX_SHADER, "Matrix");
	miniMapRender_->CreateCBV(sizeof(int), ShaderType::PIXEL_SHADER, "TextureIndex");
	miniMapRender_->SetUseTexture(true);
	miniMapRender_->psoConfig_.vs = "Simple.VS.hlsl";
	miniMapRender_->psoConfig_.ps = "PostEffect/Simple.PS.hlsl";

	visionField_ = std::make_unique<RenderObject>();
	visionField_->Initialize();
	visionField_->SetDrawData(visionField);
	visionField_->CreateCBV(sizeof(Matrix4x4), ShaderType::VERTEX_SHADER, "WVP");
	visionField_->CreateCBV(sizeof(int), ShaderType::PIXEL_SHADER, "TextureIndex");
	visionField_->SetUseTexture(true);
	visionField_->psoConfig_.vs = "Simple.VS.hlsl";
	visionField_->psoConfig_.ps = "PostEffect/Simple.PS.hlsl";
	vfModelsTextureIndex_ = textureManager->LoadTexture("Mineral-0.png");

	bg_ = std::make_unique<RenderObject>();
	bg_->Initialize();
	bg_->SetDrawData(plane);
	bg_->CreateCBV(sizeof(Matrix4x4), ShaderType::VERTEX_SHADER, "Matrix");
	bg_->CreateCBV(sizeof(int), ShaderType::PIXEL_SHADER, "TextureIndex");
	bg_->SetUseTexture(true);
	bg_->psoConfig_.vs = "Simple.VS.hlsl";
	bg_->psoConfig_.ps = "PostEffect/Simple.PS.hlsl";
	bgti_ = textureManager->LoadTexture("MiniMapBG.png");
}

void MiniMap::Update(bool debug) {
#ifdef SH_RELEASE
	screenMousePos_ = DebugMousePos::screenMousePos;
#endif
#ifdef USE_IMGUI
	screenMousePos_ = DebugMousePos::gameMousePos;
#endif

	if (!debug) {
		if (Input::GetMouseButtonState()[0] && !Input::GetPreMouseButtonState()[0] && DebugMousePos::isHovered) {
			Vector2 deadZone = { 10.0f, 10.0f };
			Vector2 min = Vector2(960, 540) + deadZone;
			Vector2 max = Vector2(1280, 720);

			//MiniMapがクリックされたら
			if (screenMousePos_.x >= min.x && screenMousePos_.x <= max.x && screenMousePos_.y >= min.y && screenMousePos_.y <= max.y) {
				pleasePose_ = !pleasePose_;
			} else {


				pleasePose_ = false;
			}

			// コールバック関数
			onClicked_();
		}
	}

	Vector3 cameraPos = transform_.position + Vector3{ 0.0f, dist * distRatio_, 0.0f };
	camera_->SetTransform(Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, { cameraPos }).Inverse());
	camera_->MakeMatrix();

	auto key = Input::GetKeyState();
	auto prekey = Input::GetPreKeyState();

	if (!debug) {
		//特定のキーが押されたらモードを切り替える
		if ((key[DIK_SPACE] && !prekey[DIK_SPACE]) || (key[DIK_M] && !prekey[DIK_M]) || (key[DIK_F4] && !prekey[DIK_F4])) {
			pleasePose_ = !pleasePose_;
			// コールバック関数
			onClicked_();
		}
	}
}

Camera* MiniMap::PreDraw(Window* window) {
	display_->PreDraw(window->GetCommandObject(), true);

	return camera_.get();
}

void MiniMap::PostDraw(Window* window, const Matrix4x4& vpMatrix, Vector3 playerPosition, float range) {
	//ポーズ中は描画しない
	if (!pleasePose_) {
		//枠を描画
		float scale = range / rangeAdjust_;
		Vector3 pos = { playerPosition.x, 0.1f, playerPosition.z };
		Matrix4x4 mat = Matrix::MakeAffineMatrix({ scale, scale, scale }, {}, pos) * vpMatrix;
		visionField_->CopyBufferData(0, &mat, sizeof(Matrix4x4));
		visionField_->CopyBufferData(1, &vfModelsTextureIndex_, sizeof(int));
		visionField_->Draw(window);
	}

	display_->PostDraw(window->GetCommandObject());
}

void MiniMap::Draw(Window* window) {

	Vector3 scale = { 0.5f, 0.5f, 1.0f };
	Vector3 pos = { 0.75f, -0.75f, 0.0f };
	if (pleasePose_) {
		scale = {2.0f, 2.0f, 1.0f};
		pos = { 0.0f, 0.0f, 0.0f };
	}

	//背景描画
	Matrix4x4 matWorld = Matrix::MakeAffineMatrix(scale, { 0.0f, 0.0f, 1.57f }, pos);
	Matrix4x4 mat = Matrix::MakeAffineMatrix({scale.x + 0.1f, scale.y, scale.z}, {0.0f, 0.0f, 1.57f}, pos);
	bg_->CopyBufferData(0, &mat, sizeof(Matrix4x4));
	bg_->CopyBufferData(1, &bgti_, sizeof(int));
	//bg_->Draw(window);

	miniMapRender_->CopyBufferData(0, &matWorld, sizeof(Matrix4x4));
	int textureIndex = display_->GetTextureData()->GetOffset();
	miniMapRender_->CopyBufferData(1, &textureIndex, sizeof(int));

	miniMapRender_->Draw(window);
}

void MiniMap::DrawImGui() {
#ifdef USE_IMGUI

	ImGui::Begin("MiniMap");

	TextureData* textureData = display_->GetTextureData();
	ImGui::Image((void*)(intptr_t)textureData->GetGPUHandle().ptr, ImVec2(400, 225));
	ImGui::DragFloat("Distance Ratio", &distRatio_, 0.01f);
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.1f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.1f);
	ImGui::DragFloat3("Position", &transform_.position.x, 0.1f);
	Vector3 cameraPos = transform_.position + Vector3{ 0.0f, dist * distRatio_, 0.0f };
	camera_->SetTransform(Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, { cameraPos }).Inverse());
	camera_->MakeMatrix();

	ImGui::End();

#endif
}

Vector3 MiniMap::WorldToScreen(const Vector3& worldPos) const {
	// VP行列を取得
	Matrix4x4 mat = camera_->GetVPMatrix();

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

MarkerResult MiniMap::GetMarkerInfo(const Vector3& targetWorldPos, float margin, float screenWidth, float screenHeight) {
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