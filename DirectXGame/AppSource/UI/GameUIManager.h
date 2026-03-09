#pragma once
#include"Object/FontObject.h"
#include"Object/SpriteObject.h"
#include"UI/CounterUI.h"
#include"UI/TimerUI.h"
#include"Effect/QuotaClearEffectUI.h"
#include"Effect/CollectEffectUI.h"

class GameUIManager {
public:

	void Initialize(DrawData spriteData,int starTexture,int lineTexture,int oreIcon,int itemIcon,int clockIcon,const std::string& fontName, DrawData fontData, FontLoader* fontLoader,int florNum);

	void Update(const int32_t& unitNum,const int32_t& maxUnitNum);

	void Draw(Window* window, const Matrix4x4& vpMatrix, bool isDrawEffect);

public:

	// 前の時間を取得
	void SetPreTime(const int& time) {
		timerUI_->SetPreTime(time);
	}

	void StartAnimation() {
		unitCounterUI_->Start();
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

	// アイコン画像
	std::unique_ptr<SpriteObject> oreIconSpriteObject_;
	std::unique_ptr<SpriteObject> itemIconSpriteObject_;
	std::unique_ptr<SpriteObject> clockIconSpriteObject_;

	// 階層名前
	std::unique_ptr<FontObject> floorFontObject_;

	// ノルマ文字
	std::unique_ptr<FontObject> quotaFontObject_;

	// ユニット文字
	std::unique_ptr<FontObject> unitFontObject_;

	// ノルマクリア演出
	std::unique_ptr<QuotaClearEffectUI> quotaClearEffectUI_;

	// 収集演出
	std::unique_ptr<CollectEffectUI> collectEffectUI_;

	float alertTImer_ = 0.0f;
	
	uint32_t timeSH_ = 0;
	bool isPlaySH_ = false;

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