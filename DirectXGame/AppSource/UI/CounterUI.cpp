#include"CounterUI.h"
#include"FpsCount.h"
#include"Utility/Easing.h"

void CounterUI::Initialize(const std::string& fontName, int32_t curNum, int32_t maxNum, DrawData drawData, FontLoader* fontLoader) {

	// フォントローダーを取得
	fontLoader_ = fontLoader;

	// 数を取得
	currenyNum_ = curNum;
	maxNum_ = maxNum;

	// テキストの初期化
	fontObject_ = std::make_unique<FontObject>();
	fontObject_->Initialize(fontName, std::to_wstring(curNum), drawData, fontLoader);

	std::wstring s = +L"/" + std::to_wstring(maxNum);

	// テキストの初期化
	maxnumFontObject_ = std::make_unique<FontObject>();
	maxnumFontObject_->Initialize(fontName, s, drawData, fontLoader);
	maxnumFontObject_->transform_.scale = { 0.8f,-0.8f,1.0f };
}

void CounterUI::Update(int32_t curNum, int32_t maxNum) {

	fontObject_->transform_.scale = size_;

	if (curNum != currenyNum_ || maxNum != maxNum_) {

		currenyNum_ = curNum;
		maxNum_ = maxNum;

		isAnimation_ = true;

		// カウントによりテクスチャを更新
		fontObject_->UpdateCharPositions(std::to_wstring(curNum), fontLoader_);
		maxnumFontObject_->UpdateCharPositions(L"/" + std::to_wstring(maxNum), fontLoader_);
	}

	// アニメーションの更新処理
	if (isAnimation_) {
		Animation();
	}

	// 1つの文字サイズ
	textSize_ = fontObject_->transform_.scale.x * 64.0f;

	// 位置の更新
	int digitCount = (currenyNum_ >= 10) ? 2 : 1;
	float totalWidth = digitCount * textSize_;
	float totalHeight = textSize_;

	float offsetX = totalWidth * 0.5f;
	float offsetY = totalHeight * 0.5f;

	fontObject_->transform_.position.x = pos_.x - offsetX;
	fontObject_->transform_.position.y = pos_.y - offsetY;

	float maxTextSize = (maxnumFontObject_->transform_.scale.x * 64.0f) * 0.5f;
	maxnumFontObject_->transform_.position = pos_;
	maxnumFontObject_->transform_.position.x = fontObject_->transform_.position.x + offsetX + textSize_*0.4f;
	maxnumFontObject_->transform_.position.y = pos_.y - maxTextSize;
}

void CounterUI::Draw(Window* window, const Matrix4x4& vpMatrix) {
	
	// フォントを描画
	fontObject_->Draw(window, vpMatrix);
	maxnumFontObject_->Draw(window, vpMatrix);
}

void CounterUI::Animation() {

	timer_ += FpsCount::deltaTime / maxTime_;

	if (timer_ <= 0.5f) {
		float localT = timer_ / 0.5f;

		// スケール
		float width = lerp(0.0f, 0.2f, localT, EaseType::EaseInCubic);
		fontObject_->transform_.scale.x = size_.x + width;
		fontObject_->transform_.scale.y = size_.y - width;

	} else {
		float localT = (timer_ - 0.5f) / 0.5f;

		// スケール
		float width = lerp(0.2f, 0.0f, localT, EaseType::EaseInCubic);
		fontObject_->transform_.scale.x = size_.x + width;
		fontObject_->transform_.scale.y = size_.y - width;
	}

	if (timer_ >= 1.0f) {
		isAnimation_ = false;
		timer_ = 0.0f;
	}
}