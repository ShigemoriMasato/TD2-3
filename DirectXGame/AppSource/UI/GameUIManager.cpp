#include"GameUIManager.h"
#include"Item/OreItemStorageNum.h"
#include <Common/DebugParam/GameParamEditor.h>

void GameUIManager::Initialize(DrawData spriteData, const std::string& fontName, DrawData fontData, FontLoader* fontLoader) {
	fontLoader_ = fontLoader;

	// ユニットの数UI
	unitCounterUI_ = std::make_unique<CounterUI>();
	unitCounterUI_->Initialize(fontName, 0, 10, fontData, fontLoader_);
	unitCounterUI_->pos_.x = 800.0f;
	unitCounterUI_->pos_.y = 128.0f;

	// 鉱石のアイテムUI
	oreItemUI_ = std::make_unique<CounterUI>();
	oreItemUI_->Initialize(fontName, OreItemStorageNum::currentOreItemNum_, OreItemStorageNum::maxOreItemNum_, fontData, fontLoader_);
	oreItemUI_->pos_.x = 800.0f;
	oreItemUI_->pos_.y = 200.0f;

	// 時間を表示するUI
	timerUI_ = std::make_unique<TimerUI>();
	timerUI_->Initialize(fontName, fontData, fontLoader_);

	// 描画機能の初期化
	upSpriteObject_ = std::make_unique<SpriteObject>();
	upSpriteObject_->Initialize(spriteData, { 360.0f,96.0f });
	upSpriteObject_->transform_.position = { 640.0f,360.0f,0.0f };
	upSpriteObject_->color_ = { 0.1f,0.1f,0.1f,1.0f };
	upSpriteObject_->Update();

	// 描画機能の初期化
	downSpriteObject_ = std::make_unique<SpriteObject>();
	downSpriteObject_->Initialize(spriteData, { 360.0f,96.0f });
	downSpriteObject_->transform_.position = { 640.0f,360.0f,0.0f };
	downSpriteObject_->color_ = { 0.1f,0.1f,0.1f,1.0f };
	downSpriteObject_->Update();

	floorFontObject_ = std::make_unique<FontObject>();
	floorFontObject_->Initialize(fontName, L"Wave1", fontData, fontLoader);

	quotaFontObject_ = std::make_unique<FontObject>();
	quotaFontObject_->Initialize(fontName, L"ノルマ", fontData, fontLoader);

	unitFontObject_ = std::make_unique<FontObject>();
	unitFontObject_->Initialize(fontName, L"ユニット", fontData, fontLoader);

#ifdef USE_IMGUI
	RegisterDebugParam();
#endif
	ApplyDebugParam();
}

void GameUIManager::Update(const int32_t& unitNum, const int32_t& maxUnitNum) {
#ifdef USE_IMGUI
	ApplyDebugParam();
#endif

	// ユニットの数を更新
	unitCounterUI_->Update(unitNum, maxUnitNum);

	// 鉱石の数を更新
	oreItemUI_->Update(OreItemStorageNum::currentOreItemNum_, OreItemStorageNum::maxOreItemNum_);

	// 時間表示UIを更新
	timerUI_->Update();
}

void GameUIManager::Draw(Window* window, const Matrix4x4& vpMatrix) {

	// 背景
	upSpriteObject_->Draw(window, vpMatrix);
	downSpriteObject_->Draw(window, vpMatrix);

	// 説明文字を描画
	quotaFontObject_->Draw(window, vpMatrix);
	floorFontObject_->Draw(window, vpMatrix);
	unitFontObject_->Draw(window, vpMatrix);

	// ユニットの数UIを描画
	unitCounterUI_->Draw(window, vpMatrix);
	// 鉱石の数UIを描画
	oreItemUI_->Draw(window, vpMatrix);

	// 時間計測表示UI
	timerUI_->Draw(window, vpMatrix);
}

void GameUIManager::RegisterDebugParam() {
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "UpPos", upSpriteObject_->transform_.position,0);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "UpScale", upSpriteObject_->transform_.scale,1);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "DownPos", downSpriteObject_->transform_.position,2);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "DownScale", downSpriteObject_->transform_.scale,3);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "BgColor", bgColor_,4);

	GameParamEditor::GetInstance()->AddItem("GameUIManager", "UnitUIPos", unitCounterUI_->pos_,5);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "UnitUISize", unitCounterUI_->size_,6);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "QuotaUIPos", oreItemUI_->pos_,7);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "QuotaUISize", oreItemUI_->size_,8);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "TimeUIPos", timerUI_->pos_,9);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "TimeUISize", timerUI_->size_,10);

	int i = 11;
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "FloorFontPos", floorFontObject_->transform_.position, i++);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "FloorFontSize", floorFontObject_->transform_.scale, i++);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "QuotaFontPos", quotaFontObject_->transform_.position, i++);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "QuotaFontSize", quotaFontObject_->transform_.scale, i++);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "UnitFontPos", unitFontObject_->transform_.position, i++);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "UnitFontSize", unitFontObject_->transform_.scale, i++);
}

void GameUIManager::ApplyDebugParam() {
	upSpriteObject_->transform_.position = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "UpPos");
	upSpriteObject_->transform_.scale = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "UpScale");
	downSpriteObject_->transform_.position = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "DownPos");
	downSpriteObject_->transform_.scale = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "DownScale");
	bgColor_ = GameParamEditor::GetInstance()->GetValue<Vector4>("GameUIManager", "BgColor");
	upSpriteObject_->color_ = bgColor_;
	downSpriteObject_->color_ = bgColor_;

	unitCounterUI_->pos_ = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "UnitUIPos");
	unitCounterUI_->size_ = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "UnitUISize");
	oreItemUI_->pos_ = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "QuotaUIPos");
	oreItemUI_->size_ = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "QuotaUISize");
	timerUI_->pos_ = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "TimeUIPos");
	timerUI_->size_ = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "TimeUISize");

	floorFontObject_->transform_.position = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "FloorFontPos");
	floorFontObject_->transform_.scale = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "FloorFontSize");
	quotaFontObject_->transform_.position = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "QuotaFontPos");
	quotaFontObject_->transform_.scale = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "QuotaFontSize");
	unitFontObject_->transform_.position = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "UnitFontPos");
	unitFontObject_->transform_.scale = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "UnitFontSize");

	upSpriteObject_->Update();
	downSpriteObject_->Update();
}