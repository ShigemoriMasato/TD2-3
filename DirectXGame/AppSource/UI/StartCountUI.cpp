#include"StartCountUI.h"
#include"FpsCount.h"
#include"Utility/Easing.h"
#include <Common/DebugParam/GameParamEditor.h>
#include"TimeLimit.h"
#include<numbers>

void StartCountUI::Initialize(const std::string& fontName, DrawData drawData, FontLoader* fontLoader) {

	fontLoader_ = fontLoader;

	// 開始ウェーブ文字
	waveFontObject_ = std::make_unique<FontObject>();
	waveFontObject_->Initialize(fontName,L"Wave1", drawData, fontLoader);
	waveFontObject_->fontColor_ = {0.0f,1.0f,0.4980f,1.0f};

	// スタート時フォント
	startFontObject_ = std::make_unique<FontObject>();
	startFontObject_->Initialize(fontName, L"Ready", drawData, fontLoader);
	startFontObject_->fontColor_ = { 0.0f,1.0f,0.4980f,1.0f };

	// 最後にカウントする数
	numFontObject_ = std::make_unique<FontObject>();
	numFontObject_->Initialize(fontName, L"9", drawData, fontLoader);
	numFontObject_->fontColor_ = { 0.0f,1.0f,0.4980f,0.8f };

#ifdef USE_IMGUI
	RegisterDebugParam();
#endif
	ApplyDebugParam();
	numFontObject_->transform_.scale = { 5.0f,-5.0f,1.0f };
}

void StartCountUI::Update() {
#ifdef USE_IMGUI
	//ApplyDebugParam();
#endif

	// スタート時のアニメーション
	if (isStart_) {
		StartAnimation();
	}
	
	if (TimeLimit::totalSeconds <= 10) {
	
		if (preTime_ != TimeLimit::totalSeconds) {
			if (!isEndCountAnimation_) {
				numFontObject_->UpdateCharPositions(std::to_wstring(TimeLimit::totalSeconds), fontLoader_);
				isEndCountAnimation_ = true;
	
				if (TimeLimit::totalSeconds % 2 != 0) {
					numFontObject_->transform_.rotate.z = 0.3f;
					numFontObject_->transform_.position = { 522.0f,447.0f,0.0f };
				} else {
					numFontObject_->transform_.rotate.z = -0.3f;
					numFontObject_->transform_.position = { 570.0f,521.0f,0.0f };
				}
	
				if (TimeLimit::totalSeconds == 10) {
					//numFontObject_->transform_.rotate.z = 0.3f;
					//numFontObject_->transform_.position = { 465.0f,426.0f,0.0f };
					numFontObject_->transform_.rotate.z = -0.3f;
					numFontObject_->transform_.position = { 530.0f,515.0f,0.0f };
				}
			}	
		}
	
		if (isEndCountAnimation_) {
			EndAnimation();
		}	
	}
}

void StartCountUI::Draw(Window* window, const Matrix4x4& vpMatrix) {

	if (isStart_) {
		waveFontObject_->Draw(window, vpMatrix);

		startFontObject_->Draw(window, vpMatrix);
	} else if (isEndCountAnimation_) {
		numFontObject_->Draw(window, vpMatrix);
	}	
}

void StartCountUI::StartAnimation() {

	timer_ += FpsCount::deltaTime / startTime_;

	if (timer_ <= 0.5f) {
		float localT = timer_ / 0.5f;

		startFontObject_->transform_.position.x = lerp(1280.0f, 444.0f, localT, EaseType::EaseOutBack);

	} else if(timer_ <= 0.8f) {
		float localT = (timer_ - 0.5f) / 0.3f;

		startFontObject_->fontColor_.w = lerp(1.0f, 0.0f, localT, EaseType::EaseInOutCubic);

	} else if(timer_ <= 0.9) {

		if (!isChange_) {
			// 文字を切り替える
			startFontObject_->UpdateCharPositions(L"GO!", fontLoader_);
			startFontObject_->fontColor_.w = 1.0f;
			waveFontObject_->fontColor_.w = 0.0f;
			isChange_ = true;
		}

		float localT = (timer_ - 0.8f) / 0.1f;

		float width = lerp(4.0f, 2.0f, localT, EaseType::EaseOutCubic);
		startFontObject_->transform_.scale.x = width;
		startFontObject_->transform_.scale.y = -width;

		startFontObject_->transform_.position.x = 640.0f - (width * 32.0f * 3.0f * 0.5f);
		startFontObject_->transform_.position.y = 360.0f + (width * 24.0f * 3.0f * 0.5f);
	} else {
		float localT = (timer_ - 0.9f) / 0.1f;
		startFontObject_->fontColor_.w = lerp(1.0f, 0.0f, localT, EaseType::EaseInOutCubic);
	}

	if (timer_ >= 1.0f) {
		isStart_ = false;
		isStartAnimeEnd_ = true;
		timer_ = 0.0f;
		startFontObject_->transform_.scale.x = 0.0f;
		startFontObject_->transform_.scale.z = 0.0f;
	}
}

void StartCountUI::EndAnimation() {

	timer_ += FpsCount::deltaTime / countTime_;

	if (timer_ <= 0.2f) {

	} else {
		float localT = (timer_ - 0.2f) / 0.8f;

		numFontObject_->fontColor_.w = lerp(0.8f,0.0f,localT,EaseType::EaseInCubic);
	}

	if (timer_ >= 1.0f) {
		timer_ = 0.0f;
		numFontObject_->fontColor_.w = 1.0f;
		isEndCountAnimation_ = false;
	}
}

void StartCountUI::RegisterDebugParam() {

	int i = 0;
	GameParamEditor::GetInstance()->AddItem("StartCountUI", "WaveFontPos", waveFontObject_->transform_.position, i++);
	GameParamEditor::GetInstance()->AddItem("StartCountUI", "WaveFontSize", waveFontObject_->transform_.scale, i++);

	GameParamEditor::GetInstance()->AddItem("StartCountUI", "StartFontPos", startFontObject_->transform_.position, i++);
	GameParamEditor::GetInstance()->AddItem("StartCountUI", "StartFontSize", startFontObject_->transform_.scale, i++);

	GameParamEditor::GetInstance()->AddItem("StartCountUI", "NumFontPos", numFontObject_->transform_.position, i++);
	GameParamEditor::GetInstance()->AddItem("StartCountUI", "NumFontSize", numFontObject_->transform_.scale, i++);

	GameParamEditor::GetInstance()->AddItem("StartCountUI", "r", rotZ_, i++);

}

void StartCountUI::ApplyDebugParam() {

	waveFontObject_->transform_.position = GameParamEditor::GetInstance()->GetValue<Vector3>("StartCountUI", "WaveFontPos");
	waveFontObject_->transform_.scale = GameParamEditor::GetInstance()->GetValue<Vector3>("StartCountUI", "WaveFontSize");

	startFontObject_->transform_.position = GameParamEditor::GetInstance()->GetValue<Vector3>("StartCountUI", "StartFontPos");
	startFontObject_->transform_.scale = GameParamEditor::GetInstance()->GetValue<Vector3>("StartCountUI", "StartFontSize");

	numFontObject_->transform_.position = GameParamEditor::GetInstance()->GetValue<Vector3>("StartCountUI", "NumFontPos");
	//numFontObject_->transform_.scale = GameParamEditor::GetInstance()->GetValue<Vector3>("StartCountUI", "NumFontSize");

	rotZ_ = GameParamEditor::GetInstance()->GetValue<float>("StartCountUI", "r");
	numFontObject_->transform_.rotate.z = rotZ_;
}