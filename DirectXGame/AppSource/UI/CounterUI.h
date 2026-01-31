#pragma once
#include"Object/FontObject.h"
#include<array>

class CounterUI {
public:

	void Initialize(const std::string& fontName,int32_t curNum,int32_t maxNum, DrawData drawData, FontLoader* fontLoader);

	void Update(int32_t curNum, int32_t maxNum);

	void Draw(Window* window, const Matrix4x4& vpMatrix);

public:

	Vector3 pos_;

	Vector3 size_ = { 1.0f,-1.0f,1.0f };

private:

	std::unique_ptr<FontObject> fontObject_;

	// 最大数
	std::unique_ptr<FontObject> maxnumFontObject_;

	// フォントロード
	FontLoader* fontLoader_ = nullptr;

	int32_t currenyNum_ = 0;
	int32_t maxNum_ = 0;

	float timer_ = 0.0f;
	float maxTime_ = 0.5f;
	bool isAnimation_ = false;

	float textSize_ = 64.0f;

private:

	// アニメーション
	void Animation();
};