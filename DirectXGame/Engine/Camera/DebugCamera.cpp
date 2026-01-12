#include "DebugCamera.h"
#include "../Input/Input.h"
#include "Camera.h"
#include <numbers>

using namespace Matrix;

void DebugCamera::Initialize() {
	center_ = {0.0f, 2.0f, 0.0f};

	scale_ = Vector3(1.0f, 1.0f, 1.0f);
	position_= Vector3(0.0f, 0.0f, -20.0f);

	spherical_.x = 20.0f;
	spherical_.y = std::numbers::pi_v<float> / 2;
	spherical_.z = -std::numbers::pi_v<float> / 2;
}

void DebugCamera::Update() {

	Vector3 cameraVelocity{};
	Vector3 centerVelocity{};

	//===================
	//球面座標系
	//===================
	Vector2 mouseMove{};
	if (Input::GetMouseButtonState()[1] || Input::GetMouseButtonState()[2]) {
		mouseMove = Input::GetMouseMove();
		mouseMove.y *= -1.0f; // Y軸を反転
	}
	float mouseWheel = -Input::GetMouseWheel();

	if (Input::GetMouseButtonState()[2] || Input::GetKeyState(DIK_LSHIFT)) {
		float speed = spherical_.x * 0.05f;
		center_ -= Vector3(mouseMove.x * speed_, mouseMove.y * speed_, mouseWheel * 0.05f) * speed * MakeRotationMatrix(rotation_);
	} else {
		spherical_ += Vector3(mouseWheel * 0.05f, mouseMove.y * speed_, -mouseMove.x * speed_);
	}

	spherical_.x = std::max(0.01f, spherical_.x); // マイナスにならないようにする

	position_.x = std::sinf(spherical_.y) * std::cosf(spherical_.z);
	position_.y = std::cosf(spherical_.y);
	position_.z = std::sinf(spherical_.y) * std::sinf(spherical_.z);

	position_*= spherical_.x; // 半径を適用

	rotation_ = { -spherical_.y + std::numbers::pi_v<float> / 2, -spherical_.z - std::numbers::pi_v<float> / 2, 0.0f };
	
	//===================
	//座標の適用
	//===================
	position_ += center_;
	SetTransform(MakeTranslationMatrix(position_).Inverse() * MakeRotationMatrix(rotation_).Inverse() * MakeScaleMatrix(scale_));
	SetProjectionMatrix(PerspectiveFovDesc());
	MakeMatrix();

	// Actual camera position in world space
}

Vector3 DebugCamera::GetCenter() const {
	return -center_;
}
