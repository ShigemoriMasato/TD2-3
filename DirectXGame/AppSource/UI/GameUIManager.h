#pragma once
#include"Object/FontObject.h"
#include"Object/SpriteObject.h"
#include"UI/CounterUI.h"
#include"UI/TimerUI.h"

class GameUIManager {
public:

	void Initialize(DrawData spriteData,const std::string& fontName, DrawData fontData, FontLoader* fontLoader);

	void Update(const int32_t& unitNum,const int32_t& maxUnitNum);

	void Draw(Window* window, const Matrix4x4& vpMatrix);

public:

	// 前の時間を取得
	void SetPreTime(const int& time) {
		timerUI_->SetPreTime(time);
	}

private:

	// フォントロード
	FontLoader* fontLoader_ = nullptr;

	// 計測時間をだすUI
	std::unique_ptr<TimerUI> timerUI_;

	// ユニットの数UI
	std::unique_ptr<CounterUI> unitCounterUI_;
	// 鉱石の数UI
	std::unique_ptr<CounterUI> oreItemUI_;

	// 背景画像
	std::unique_ptr<SpriteObject> upSpriteObject_;
	std::unique_ptr<SpriteObject> downSpriteObject_;

	Vector4 bgColor_ = { 0.0f,0.0f,0.0f,1.0f };

	// 階層名前
	std::unique_ptr<FontObject> floorFontObject_;

	// ノルマ文字
	std::unique_ptr<FontObject> quotaFontObject_;

	// ユニット文字
	std::unique_ptr<FontObject> unitFontObject_;

private:

	/// <summary>
	/// 値を登録する
	/// </summary>
	void RegisterDebugParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam();
};