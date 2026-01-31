#include"TimerUI.h"
#include"TimeLimit.h"
#include"FpsCount.h"
#include"Utility/Easing.h"

void TimerUI::Initialize(const std::string& fontName, DrawData drawData, FontLoader* fontLoader) {
	// フォントローダーを取得
	fontLoader_ = fontLoader;

	// フォント
	numFont_ = std::make_unique<FontObject>();
	numFont_->Initialize(fontName, std::to_wstring(TimeLimit::totalSeconds), drawData, fontLoader);
	numFont_->transform_.position.x = pos_.x;
	numFont_->transform_.position.y = pos_.y;
}

void TimerUI::Update() {

	// 桁を求める
	digits_ = 0;
	int n = std::abs(TimeLimit::totalSeconds);
	do {
		digits_++;
		n /= 10;
	} while (n != 0);

	numFont_->transform_.scale = size_;
	numFont_->transform_.position = pos_;

	// 時間文字の更新処理
	numFont_->UpdateCharPositions(std::to_wstring(TimeLimit::totalSeconds), fontLoader_);

	// 文字をアニメーション
	if (TimeLimit::totalSeconds <= 30) {

		if (preTime_ != TimeLimit::totalSeconds) {
			isAnimation_ = true;
		}

		// アニメーションの更新処理
		if (isAnimation_) {
			Animation();
		}
	}
}

void TimerUI::Draw(Window* window, const Matrix4x4& vpMatrix) {
	// 数字を描画する
	numFont_->Draw(window, vpMatrix);
}

void TimerUI::Animation() {

	timer_ += FpsCount::deltaTime / maxTime_;

	if (timer_ <= 0.5f) {
		float localT = timer_ / 0.5f;

		// スケール
		float width = lerp(0.0f, 0.5f, localT, EaseType::EaseInCubic);
		numFont_->transform_.scale.x = size_.x + width;
		numFont_->transform_.scale.y = size_.y - width;
		// フォントの色
		numFont_->fontColor_ = lerp(fontColor_,Vector4(1.0f,1.0f,1.0f,1.0f),localT,EaseType::EaseInCubic);

		// 座標
		numFont_->transform_.position.x = lerp(pos_.x, pos_.x - (32.0f * 0.5f), localT, EaseType::EaseInCubic);
	} else {
		float localT = (timer_ - 0.5f) / 0.5f;

		// スケール
		float width = lerp(0.5f, 0.0f, localT, EaseType::EaseInCubic);
		numFont_->transform_.scale.x = size_.x + width;
		numFont_->transform_.scale.y = size_.y - width;
		// フォントの色
		numFont_->fontColor_ = lerp(Vector4(1.0f, 1.0f, 1.0f, 1.0f),fontColor_ , localT, EaseType::EaseInCubic);

		// 座標
		numFont_->transform_.position.x = lerp(pos_.x - (32.0f * 0.5f), pos_.x, localT, EaseType::EaseInCubic);
	}

	if (timer_ >= 1.0f) {
		isAnimation_ = false;
		timer_ = 0.0f;
	}
}