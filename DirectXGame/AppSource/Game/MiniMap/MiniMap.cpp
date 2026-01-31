#include "MiniMap.h"
#include <numbers>
#include <GameCamera/DebugMousePos.h>
#include <Input/Input.h>

void MiniMap::Initialize(int mapWidth, int mapHeight, TextureManager* textureManager, const DrawData& plane, const DrawData& visionField) {
	// カメラの初期化
	camera_ = std::make_unique<Camera>();
	camera_->SetProjectionMatrix(PerspectiveFovDesc());

	transform_.rotate.x = std::numbers::pi_v<float> / 2.0f;
	distRatio_ = 0.7f;

	dist = std::max(mapWidth, mapHeight) * 0.5f / std::tan(0.45f / 2.0f);
	transform_.position = { mapWidth / 2.0f - 0.5f, 0.0f, mapHeight / 2.0f - 0.5f };
	Vector3 cameraPos = transform_.position + Vector3{ 0.0f, dist * distRatio_, 0.0f };
	camera_->SetTransform(Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, { cameraPos }));
	camera_->MakeMatrix();
	// デュアルディスプレイの初期化
	display_ = std::make_unique<DualDisplay>("MiniMapDisplay");
	int textureHandle1 = textureManager->CreateWindowTexture(1280, 720, 0x000000ff);
	int textureHandle2 = textureManager->CreateWindowTexture(1280, 720, 0x000000ff);
	display_->Initialize(textureManager->GetTextureData(textureHandle1), textureManager->GetTextureData(textureHandle2));

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
}

void MiniMap::Update() {
#ifdef SH_RELEASE
	screenMousePos_ = DebugMousePos::screenMousePos;
#endif
#ifdef USE_IMGUI
	screenMousePos_ = DebugMousePos::gameMousePos;
#endif

	if (Input::GetMouseButtonState()[0] && !Input::GetPreMouseButtonState()[0]) {
		Vector2 deadZone = { 10.0f, 10.0f };
		Vector2 min = Vector2(960, 540) + deadZone;
		Vector2 max = Vector2(1280, 720);

		//Minimapがクリックされたら
		if (screenMousePos_.x >= min.x && screenMousePos_.x <= max.x && screenMousePos_.y >= min.y && screenMousePos_.y <= max.y) {
			pleasePose_ = !pleasePose_;
		} else {
			pleasePose_ = false;
		}
	}

	auto key = Input::GetKeyState();
	auto prekey = Input::GetPreKeyState();

	//特定のキーが押されたらモードを切り替える
	if ((key[DIK_TAB] && !prekey[DIK_TAB]) || (key[DIK_M] && prekey[DIK_M]) || (key[DIK_F4] && !prekey[DIK_F4])) {
		pleasePose_ = !pleasePose_;
	}
}

Camera* MiniMap::PreDraw(Window* window) {
	display_->PreDraw(window->GetCommandObject(), true);
	return camera_.get();
}

void MiniMap::PostDraw(Window* window, const Matrix4x4& vpMatrix, Vector3 playerPosition, float range) {
	//枠を描画
	float scale = range / rangeAdjust_;
	Vector3 pos = { playerPosition.x, 0.1f, playerPosition.z };
	Matrix4x4 mat = Matrix::MakeAffineMatrix({ scale, scale, scale }, {}, pos) * vpMatrix;
	visionField_->CopyBufferData(0, &mat, sizeof(Matrix4x4));
	visionField_->CopyBufferData(1, &vfModelsTextureIndex_, sizeof(int));
	visionField_->Draw(window);

	display_->PostDraw(window->GetCommandObject());
}

void MiniMap::Draw(Window* window) {

	float scale = 0.5f;
	Vector3 pos = { 0.75f, -0.75f, 0.0f };
	if (pleasePose_) {
		scale = 2.0f;
		pos = { 0.0f, 0.0f, 0.0f };
	}

	Matrix4x4 matWorld = Matrix::MakeAffineMatrix({ scale, scale, scale }, { 0.0f, 0.0f, 1.57f }, pos);
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
