#include"PauseUI.h"
#include"Utility/Easing.h"
#include"FpsCount.h"
#include"Assets/Audio/AudioManager.h"
#include"GameCamera/DebugMousePos.h"

void PauseUI::Initialize(DrawData drawData, uint32_t texture, KeyManager* keyManager,
	const std::string& fontName, DrawData fontDrawData, FontLoader* fontLoader,
	int baTex, int guidTex, int selTex, int log) {
	keyManager_ = keyManager;

	// 描画機能の初期化
	bgSpriteObject_ = std::make_unique<SpriteObject>();
	bgSpriteObject_->Initialize(drawData, { 520.0f,720.0f });
	bgSpriteObject_->transform_.position = { 640.0f,360.0f,0.0f };
	bgSpriteObject_->color_ = { 0.0f,0.0f,0.0f,0.7f };
	bgSpriteObject_->SetTexture(0);
	bgSpriteObject_->Update();

	fliterSpriteObject_ = std::make_unique<SpriteObject>();
	fliterSpriteObject_->Initialize(drawData, { 1280.0f,720.0f });
	fliterSpriteObject_->transform_.position = { 640.0f,360.0f,0.0f };
	fliterSpriteObject_->color_ = { 0.0f,0.0f,0.0f,0.5f };
	fliterSpriteObject_->SetTexture(0);
	fliterSpriteObject_->Update();

	// ポーズのロゴ描画用
	logSprite_ = std::make_unique<SpriteObject>();
	logSprite_->Initialize(drawData, { 420.0f,256.0f });
	logSprite_->transform_.position = { -256.0f,160.0f,0.0f };
	logSprite_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	logSprite_->SetTexture(log);
	logSprite_->Update();

	// テキストの初期化
	gameOverFontObject_ = std::make_unique<FontObject>();
	gameOverFontObject_->Initialize(fontName, L"Tab : ポーズ", fontDrawData, fontLoader);
	//gameOverFontObject_->transform_.position.x = 640.0f - (static_cast<float>(gameOverFontObject_->GetTextSize()) * 48.0f * 0.5f);
	gameOverFontObject_->transform_.position.x = 32.0f;
	gameOverFontObject_->transform_.position.y = 680.0f;
	gameOverFontObject_->transform_.scale.x = 0.8f;
	gameOverFontObject_->transform_.scale.y = -0.8f;
	gameOverFontObject_->fontColor_ = { 1.0f, 0.27f, 0.0f,1.0f };

	// 選択画像を初期化
	for (size_t i = 0; i < selectSpriteObject_.size(); ++i) {
		selectSpriteObject_[i] = std::make_unique<SpriteObject>();
		selectSpriteObject_[i]->Initialize(drawData, { 256.0f,64.0f });
		selectSpriteObject_[i]->transform_.position = { -300.0f,430.0f + i * (96.0f + 8.0f) ,0.0f };
		selectSpriteObject_[i]->color_ = { 1.0f,1.0f,1.0f,0.9f };

		if (i == 0) {
			selectSpriteObject_[i]->SetTexture(baTex);
		} else if (i == 1) {
			selectSpriteObject_[i]->SetTexture(guidTex);
		} else {
			selectSpriteObject_[i]->SetTexture(selTex);
		}
		selectSpriteObject_[i]->Update();
	}

	// 操作UI
	guideSprite_ = std::make_unique<SpriteObject>();
	guideSprite_->Initialize(drawData, { 1280,720.0f });
	guideSprite_->transform_.position = { 640.0f,360.0f,0.0f };
	guideSprite_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	guideSprite_->SetTexture(texture);
	guideSprite_->Update();


	/// 音声

	// 決定音を取得
	decideSH_ = AudioManager::GetInstance().GetHandleByName("Decide.mp3");

}

void PauseUI::Update() {

	// アニメーション
	if (isAnimation_) {

		if (isOpenPause_) {
			// スタートアニメーション
			StartAnimation();
		} else {
			// エンドアニメーション
			EndAnimation();
		}
		
	} 

	InUpdate();

	// 更新処理
	for (size_t i = 0; i < selectSpriteObject_.size(); ++i) {

		if (!isAnimation_ && isOpenPause_) {

			float baseX = 96.0f;
			float offset = 32.0f;

			if (i == selectNum_) {
				selectSpriteObject_[i]->transform_.position.x = baseX + offset;
			} else {
				// 選択されていない画像は、基準位置に戻す
				selectSpriteObject_[i]->transform_.position.x = baseX;
			}
		}

		selectSpriteObject_[i]->Update();
	}

	// 背景画像の更新処理
	bgSpriteObject_->Update();
	logSprite_->Update();
}

void PauseUI::Draw(Window* window, const Matrix4x4& vpMatrix) {

	if (isOpenPause_ || isAnimation_) {

		fliterSpriteObject_->Draw(window, vpMatrix);
		// 背景の描画
		bgSpriteObject_->Draw(window, vpMatrix);

		// ロゴを描画
		logSprite_->Draw(window, vpMatrix);

		// 選択アイコン
		for (size_t i = 0; i < selectSpriteObject_.size(); ++i) {
			selectSpriteObject_[i]->Draw(window, vpMatrix);
		}
		
	} else {
		gameOverFontObject_->Draw(window, vpMatrix);
	}
}

void PauseUI::DrawGuideUI(Window* window, const Matrix4x4& vpMatrix) {

	if (isOpenPause_ || isAnimation_) {
		// 操作UIを表示
		if (isGuideOpen_) {
			guideSprite_->Draw(window, vpMatrix);
		}
	}	
}

void PauseUI::InUpdate() {

	// 操作
	auto key = keyManager_->GetKeyStates();

	if (!isGuideOpen_) {
		if (key[Key::UpTri]) {
			if (selectNum_ > 0) {
				selectNum_--;
			}
		}

		if (key[Key::DownTri]) {
			if (selectNum_ < 2) {
				selectNum_++;
			}
		}
	}

	Vector2 screenPos = {};
#ifdef USE_IMGUI
	screenPos = DebugMousePos::gameMousePos;
#else
	screenPos = DebugMousePos::screenMousePos;
#endif

	// マウス位置に夜選択
	for (size_t i = 0; i < selectSpriteObject_.size(); ++i) {
		if (selectSpriteObject_[i]->transform_.position.x - 128.0f <= screenPos.x &&
			selectSpriteObject_[i]->transform_.position.x + 128.0f >= screenPos.x) {
			if (selectSpriteObject_[i]->transform_.position.y - 32.0f <= screenPos.y &&
				selectSpriteObject_[i]->transform_.position.y + 32.0f >= screenPos.y) {
				selectNum_ = static_cast<int>(i);
			}
		}
	}

	if (isOpenPause_) {

		if (key[Key::Decision] || ((Input::GetMouseButtonState()[0] & 0x80) && !(Input::GetPreMouseButtonState()[0] & 0x80))) {

			if (!AudioManager::GetInstance().IsPlay(decideSH_)) {
				AudioManager::GetInstance().Play(decideSH_, 0.5f, false);
			}
			timer_ = 0.0f;

			// 決定
			if (selectNum_ == 0) {
				onRetryClicked_();
				isAnimation_ = true;
				isOpenPause_ = !isOpenPause_;
				// 戻る処理
				if (!isOpenPause_) {
					selectNum_ = 0;
				}
			} else if (selectNum_ == 1) {
				isGuideOpen_ = !isGuideOpen_;
			} else {
				onSelectClicked_();
			}

		}else if (key[Key::DecisionPause]) {
			if (!AudioManager::GetInstance().IsPlay(decideSH_)) {
				AudioManager::GetInstance().Play(decideSH_, 0.5f, false);
			}
			timer_ = 0.0f;
		    // tabで元にモデル
			onRetryClicked_();
			isAnimation_ = true;
			isOpenPause_ = !isOpenPause_;
			// 戻る処理
			if (!isOpenPause_) {
				selectNum_ = 0;
			}
		}
	} else {
		// 決定
		 if (key[Key::DecisionPause]) {
			if (!AudioManager::GetInstance().IsPlay(decideSH_)) {
				AudioManager::GetInstance().Play(decideSH_, 0.5f, false);
			}
			timer_ = 0.0f;
			onRetryClicked_();
			isAnimation_ = true;
			isOpenPause_ = !isOpenPause_;
		}
	}
}

void PauseUI::StartAnimation() {
	timer_ += FpsCount::deltaTime / maxTime_;

	if (timer_ <= 0.1f) {
		float localT = timer_ / 0.1f;
		//float alpha = lerp(0.0f, 0.9f, localT, EaseType::EaseOutBounce);
		// 透明度を出す
		//gameOverFontObject_->fontColor_.w = alpha;

		bgSpriteObject_->transform_.position.x = lerp(-300.0f, 350.0f, localT, EaseType::EaseOutBounce);
	}

	// ロゴ
	if (timer_ <= 0.5f) {
		float localT = timer_ / 0.5f;
		logSprite_->transform_.position.x = lerp(-256.0f, 270.0f, localT, EaseType::EaseInOutCubic);
	}

	// 選択
	if (timer_ <= 1.0f && timer_ >= 0.1f) {
		float localT = (timer_ - 0.1f) / 0.9f;
		selectSpriteObject_[0]->transform_.position.x = lerp(-300.0f, 96.0f + 32.0f, localT, EaseType::EaseOutCubic);
	}

	if (timer_ <= 1.0f && timer_ >= 0.2f) {
		float localT = (timer_ - 0.2f) / 0.8f;
		selectSpriteObject_[1]->transform_.position.x = lerp(-300.0f, 96.0f, localT, EaseType::EaseOutCubic);
	}

	if (timer_ <= 1.0f && timer_ >= 0.3f) {
		float localT = (timer_ - 0.3f) / 0.7f;
		selectSpriteObject_[2]->transform_.position.x = lerp(-300.0f, 96.0f, localT, EaseType::EaseOutCubic);
	}

	if (timer_ >= 1.0f) {
		float alpha = 1.0f;
		gameOverFontObject_->fontColor_.w = alpha;

		logSprite_->transform_.position.x = 270.0f;
		isAnimation_ = false;
	}
}

void PauseUI::EndAnimation() {
	timer_ += FpsCount::deltaTime / maxTime_;

	if (timer_ <= 0.1f) {
		float localT = timer_ / 0.1f;
		float alpha = lerp(0.0f, 0.9f, localT, EaseType::EaseOutBounce);
		// 透明度を出す
		gameOverFontObject_->fontColor_.w = alpha;

		bgSpriteObject_->transform_.position.x = lerp(350.0f, -400.0f, localT, EaseType::EaseOutBounce);
	}

	// ロゴ
	if (timer_ <= 0.5f) {
		float localT = timer_ / 0.5f;
		logSprite_->transform_.position.x = lerp(270.0f, -256.0f, localT, EaseType::EaseInOutCubic);
	}

	if (timer_ <= 1.0f && timer_ >= 0.1f) {
		float localT = (timer_ - 0.1f) / 0.9f;
		selectSpriteObject_[0]->transform_.position.x = lerp(96.0f, -300.0f, localT, EaseType::EaseOutCubic);
	}

	if (timer_ <= 1.0f && timer_ >= 0.2f) {
		float localT = (timer_ - 0.2f) / 0.8f;
		selectSpriteObject_[1]->transform_.position.x = lerp(96.0f, -300.0f, localT, EaseType::EaseOutCubic);
	}

	if (timer_ <= 1.0f && timer_ >= 0.3f) {
		float localT = (timer_ - 0.3f) / 0.7f;
		selectSpriteObject_[2]->transform_.position.x = lerp(96.0f, -300.0f, localT, EaseType::EaseOutCubic);
	}

	if (timer_ >= 1.0f) {
		float alpha = 0.9f;
		gameOverFontObject_->fontColor_.w = alpha;
		isAnimation_ = false;
	}
}