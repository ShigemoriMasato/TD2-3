#include"GameUIManager.h"
#include"Item/OreItemStorageNum.h"
#include <Common/DebugParam/GameParamEditor.h>
#include"TimeLimit.h"
#include"FpsCount.h"
#include"Utility/Easing.h"
#include"Assets/Audio/AudioManager.h"

void GameUIManager::Initialize(DrawData spriteData, int starTexture, int lineTexture, int oreIcon, int itemIcon, int clockIcon, const std::string& fontName, DrawData fontData, FontLoader* fontLoader, int florNum) {
	fontLoader_ = fontLoader;

	transform_.position = {0.0f,0.0f,0.0f};
	transform_.rotate = {0.0f,0.0f,0.0f};
	transform_.scale = {1.0f,1.0f,1.0f};
	worldMatrix_ = Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.position);

	// ユニットの数UI
	unitCounterUI_ = std::make_unique<CounterUI>();
	unitCounterUI_->SetParent(&worldMatrix_);
	unitCounterUI_->Initialize(fontName, 0, 10, fontData, fontLoader_);
	unitCounterUI_->pos_.x = 800.0f;
	unitCounterUI_->pos_.y = 128.0f;

	// 鉱石のアイテムUI
	oreItemUI_ = std::make_unique<CounterUI>();
	oreItemUI_->SetParent(&worldMatrix_);
	oreItemUI_->Initialize(fontName, OreItemStorageNum::currentOreItemNum_, OreItemStorageNum::maxOreItemNum_, fontData, fontLoader_);
	oreItemUI_->pos_.x = 800.0f;
	oreItemUI_->pos_.y = 200.0f;

	// 時間を表示するUI
	timerUI_ = std::make_unique<TimerUI>();
	timerUI_->SetParent(&worldMatrix_);
	timerUI_->Initialize(fontName, fontData, fontLoader_);

	// 描画機能の初期化
	upSpriteObject_ = std::make_unique<SpriteObject>();
	upSpriteObject_->SetParent(&worldMatrix_);
	upSpriteObject_->Initialize(spriteData, { 360.0f,96.0f });
	upSpriteObject_->transform_.position = { 640.0f,360.0f,0.0f };
	upSpriteObject_->color_ = { 0.1f,0.1f,0.1f,1.0f };
	upSpriteObject_->Update();

	// 描画機能の初期化
	downSpriteObject_ = std::make_unique<SpriteObject>();
	downSpriteObject_->SetParent(&worldMatrix_);
	downSpriteObject_->Initialize(spriteData, { 360.0f,96.0f });
	downSpriteObject_->transform_.position = { 640.0f,360.0f,0.0f };
	downSpriteObject_->color_ = { 0.1f,0.1f,0.1f,1.0f };
	downSpriteObject_->Update();

	// icon
	oreIconSpriteObject_ = std::make_unique<SpriteObject>();
	oreIconSpriteObject_->SetParent(&worldMatrix_);
	oreIconSpriteObject_->Initialize(spriteData, { 64.0f,64.0f });
	oreIconSpriteObject_->transform_.position = { 640.0f,360.0f,0.0f };
	oreIconSpriteObject_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	oreIconSpriteObject_->SetTexture(oreIcon);
	oreIconSpriteObject_->Update();
	// icon
	itemIconSpriteObject_ = std::make_unique<SpriteObject>();
	itemIconSpriteObject_->SetParent(&worldMatrix_);
	itemIconSpriteObject_->Initialize(spriteData, { 64.0f,64.0f });
	itemIconSpriteObject_->transform_.position = { 640.0f,360.0f,0.0f };
	itemIconSpriteObject_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	itemIconSpriteObject_->SetTexture(itemIcon);
	itemIconSpriteObject_->Update();
	// 時計icon
	clockIconSpriteObject_ = std::make_unique<SpriteObject>();
	clockIconSpriteObject_->SetParent(&worldMatrix_);
	clockIconSpriteObject_->Initialize(spriteData, { 64.0f,64.0f });
	clockIconSpriteObject_->transform_.position = { 640.0f,360.0f,0.0f };
	clockIconSpriteObject_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	clockIconSpriteObject_->SetTexture(clockIcon);
	clockIconSpriteObject_->Update();


	floorFontObject_ = std::make_unique<FontObject>();
	floorFontObject_->SetParent(&worldMatrix_);
	floorFontObject_->Initialize(fontName, L"フロア" + std::to_wstring(florNum), fontData, fontLoader);

	quotaFontObject_ = std::make_unique<FontObject>();
	quotaFontObject_->SetParent(&worldMatrix_);
	quotaFontObject_->Initialize(fontName, L"ノルマ", fontData, fontLoader);

	unitFontObject_ = std::make_unique<FontObject>();
	unitFontObject_->SetParent(&worldMatrix_);
	unitFontObject_->Initialize(fontName, L"ユニット", fontData, fontLoader);

	// ノルマクリアUI
	quotaClearEffectUI_ = std::make_unique<QuotaClearEffectUI>();
	quotaClearEffectUI_->Initialize(spriteData, starTexture);

	// 収集演出UI
	collectEffectUI_ = std::make_unique<CollectEffectUI>();
	collectEffectUI_->Initialize(spriteData, starTexture, lineTexture);

	timeSH_ = AudioManager::GetInstance().GetHandleByName("TimeNews.mp3");

#ifdef USE_IMGUI
	RegisterDebugParam();
#endif
	ApplyDebugParam();
	worldMatrix_ = Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.position);
	ApplyDebugParam();
}

void GameUIManager::Update(const int32_t& unitNum, const int32_t& maxUnitNum) {
#ifdef USE_IMGUI
	ApplyDebugParam();
#endif

	worldMatrix_ = Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.position);

	// ユニットの残り数によって色を変更
	if (unitNum <= 0) {
		unitCounterUI_->fontObject_->fontColor_ = { 0.8f,0.0f,0.0f,1.0f };
	} else if(unitNum <= 5) {
		unitCounterUI_->fontObject_->fontColor_ = { 0.8f,1.0f,0.0f,1.0f };
	} else {
		unitCounterUI_->fontObject_->fontColor_ = { 1.0f,1.0f,1.0f,1.0f };
	}

	// ユニットの数を更新
	unitCounterUI_->Update(unitNum, maxUnitNum);

	// 鉱石の数を更新
	oreItemUI_->Update(OreItemStorageNum::currentOreItemNum_, OreItemStorageNum::maxOreItemNum_);

	// ノルマの文字を変える
	if (OreItemStorageNum::currentOreItemNum_ >= OreItemStorageNum::maxOreItemNum_) {
		// ノルマクリア
		quotaFontObject_->UpdateCharPositions(L"ノルマクリア!", fontLoader_);
		quotaFontObject_->fontColor_ = { 0.8f,0.8f,0.0f,1.0f };
		quotaFontObject_->transform_.position.x = 834.0f;
		quotaClearEffectUI_->isLoop_ = true;

		upSpriteObject_->color_ = bgColor_;

		// 回収演出
		if (oreItemUI_->IsNumChanged()) {
			collectEffectUI_->StartAnimation();
		}

	} else {

		// ノルマを達成せずに、30秒経過
		if (TimeLimit::totalSeconds <= 30) {

			if (!isPlaySH_) {
				isPlaySH_ = true;
				AudioManager::GetInstance().Play(timeSH_, 0.5f, false);
			}
		
			alertTImer_ += FpsCount::deltaTime;

			if (alertTImer_ <= 0.5f) {
				float localT = alertTImer_ / 0.5f;

				upSpriteObject_->color_ = lerp(bgColor_, Vector4(0.8f,0.0f,0.0f,1.0f), localT, EaseType::EaseInCubic);

			} else {
				float localT = (alertTImer_ - 0.5f) / 0.5f;
				upSpriteObject_->color_ = lerp(Vector4(0.8f, 0.0f, 0.0f, 1.0f), bgColor_, localT, EaseType::EaseOutCubic);
			}

			if (alertTImer_ >= 1.0f) {
				alertTImer_ = 0.0f;
				upSpriteObject_->color_ = bgColor_;
			}
		}
	}

	upSpriteObject_->Update();


	// 時間表示UIを更新
	timerUI_->Update();

	// ノルマクリア演出の更新処理
	quotaClearEffectUI_->Update();

	// 回収演出の更新処理
	collectEffectUI_->Update();
}

void GameUIManager::StopUpdate() {
	alertTImer_ += FpsCount::deltaTime;

	if (alertTImer_ <= 0.5f) {
		float localT = alertTImer_ / 0.5f;

		upSpriteObject_->color_ = lerp(bgColor_, Vector4(0.8f, 0.8f, 0.0f, 1.0f), localT, EaseType::EaseInCubic);

	} else {
		float localT = (alertTImer_ - 0.5f) / 0.5f;
		upSpriteObject_->color_ = lerp(Vector4(0.8f, 0.8f, 0.0f, 1.0f), bgColor_, localT, EaseType::EaseOutCubic);
	}

	if (alertTImer_ >= 1.0f) {
		alertTImer_ = 0.0f;
		upSpriteObject_->color_ = bgColor_;
	}
	upSpriteObject_->Update();
}

void GameUIManager::Draw(Window* window, const Matrix4x4& vpMatrix, bool isDrawEffect) {

	// 背景
	upSpriteObject_->Draw(window, vpMatrix);
	downSpriteObject_->Draw(window, vpMatrix);

	// アイコン描画
	oreIconSpriteObject_->Draw(window, vpMatrix);
	itemIconSpriteObject_->Draw(window, vpMatrix);
	clockIconSpriteObject_->Draw(window, vpMatrix);

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

#ifdef USE_IMGUI
	ImGui::Begin("currentItem");
	ImGui::DragInt("cur", &OreItemStorageNum::currentOreItemNum_);
	ImGui::DragInt("CountCur", &oreItemUI_->currenyNum_);
	ImGui::End();
#endif

	if (isDrawEffect) {
		// ノルマクリア演出を描画
		quotaClearEffectUI_->Draw(window, vpMatrix);

		// 拐取演出
		collectEffectUI_->Draw(window, vpMatrix);
	}
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

	// icon
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "OreUnitIconPos", oreIconSpriteObject_->transform_.position, i++);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "OreUnitIconSize", oreIconSpriteObject_->transform_.scale, i++);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "OreItemIconPos", itemIconSpriteObject_->transform_.position, i++);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "OreItemIconSize", itemIconSpriteObject_->transform_.scale, i++);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "ClockIconPos", clockIconSpriteObject_->transform_.position, i++);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "ClockIconSize", clockIconSpriteObject_->transform_.scale, i++);

	GameParamEditor::GetInstance()->AddItem("GameUIManager", "BasePos", transform_.position, i++);
	GameParamEditor::GetInstance()->AddItem("GameUIManager", "BaseSize", transform_.scale, i++);

	GameParamEditor::GetInstance()->AddItem("QuotaClearEffect", "Pos", quotaClearEffectUI_->pos_);
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

	transform_.position = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "BasePos");
	transform_.scale = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "BaseSize");

	// icon
	oreIconSpriteObject_->transform_.position = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "OreUnitIconPos");
	oreIconSpriteObject_->transform_.scale = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "OreUnitIconSize");
	itemIconSpriteObject_->transform_.position = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "OreItemIconPos");
	itemIconSpriteObject_->transform_.scale = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "OreItemIconSize");
	clockIconSpriteObject_->transform_.position = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "ClockIconPos");
	clockIconSpriteObject_->transform_.scale = GameParamEditor::GetInstance()->GetValue<Vector3>("GameUIManager", "ClockIconSize");

	quotaClearEffectUI_->pos_ = GameParamEditor::GetInstance()->GetValue<Vector3>("QuotaClearEffect", "Pos");
	collectEffectUI_->pos_ = quotaClearEffectUI_->pos_;

	upSpriteObject_->Update();
	downSpriteObject_->Update();

	// icon
	oreIconSpriteObject_->Update();
	itemIconSpriteObject_->Update();
	clockIconSpriteObject_->Update();

	//oreItemUI_->Update(0,20);
}