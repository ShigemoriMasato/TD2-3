#pragma once
#include"Object/FontObject.h"
#include<array>

class TimerUI {
public:

	void Initialize(const std::string& fontName, DrawData drawData, FontLoader* fontLoader);

	void Update();

	void Draw(Window* window, const Matrix4x4& vpMatrix);

	void SetPreTime(const int& time) {
		preTime_ = time;
	}

public:

	Vector3 pos_ = {500.0f,128.0f,0.0f};

	Vector3 size_ = { 1.0f,-1.0f,1.0f };

private:

	// フォントロード
	FontLoader* fontLoader_ = nullptr;

	// フォント
	std::unique_ptr<FontObject> numFont_;

	// 色
	Vector4 fontColor_ = { 0.8f,0.8f,0.0f,1.0f };

	int preTime_ = 0;

	float timer_ = 0.0f;
	float maxTime_ = 0.3f;

	float offsetX_ = 0.0f;

	int32_t digits_ = 0;

	bool isAnimation_ = false;

private:

	void Animation();
};