#pragma once
#include"Object/SpriteObject.h"
#include"Common/KeyConfig/KeyManager.h"
#include"Object/FontObject.h"
#include <functional>
#include<array>

class PauseUI {
public:

	void Initialize(DrawData drawData,uint32_t texture, KeyManager* keyManager,
		const std::string& fontName, DrawData fontDrawData, FontLoader* fontLoader,
		int baTex,int guidTex,int selTex,int log,int tab);

	void Update();

	void Draw(Window* window, const Matrix4x4& vpMatrix);

	void DrawGuideUI(Window* window, const Matrix4x4& vpMatrix);

public:

	void SetOnRetryClicked(std::function<void()> cb) { onRetryClicked_ = std::move(cb); }
	void SetOnSelectClicked(std::function<void()> cb) { onSelectClicked_ = std::move(cb); }

private:
	// 入力処理
	KeyManager* keyManager_ = nullptr;

	// 背景画像
	std::unique_ptr<SpriteObject> bgSpriteObject_;


	// 背景画像
	std::unique_ptr<SpriteObject> fliterSpriteObject_;

	float timer_ = 0.0f;

	// 背景の移動処理
	float startBgPosY_ = 1080.0f;
	float endBgPosY_ = 360.0f;

	// アニメーションの判断
	bool isAnimation_ = false;

	// ポーズシーンの開閉
	bool isOpenPause_ = false;

	// 説明を描画
	bool isGuideOpen_ = false;

	// テキスト
	std::unique_ptr<FontObject> gameOverFontObject_;

	// クリックコールバック
	std::function<void()> onRetryClicked_ = nullptr;
	std::function<void()> onSelectClicked_ = nullptr;

	int selectNum_ = 0;

	bool isDraw_ = false;

	// 選択画像
	std::array<std::unique_ptr<SpriteObject>,3> selectSpriteObject_;

	// 操作ガイドUI
	std::unique_ptr<SpriteObject> guideSprite_;

	// ポーズロゴ
	std::unique_ptr<SpriteObject> logSprite_;

	// tab画像UI
	std::unique_ptr<SpriteObject> tabSprite_;
	Vector2 tabSize_ = { 180,64.0f };

	/// 音声

	// 決定音
	uint32_t decideSH_ = 0;

private: // 調整項目

	float maxTime_ = 0.5f;

private:

	void InUpdate();

	// 最初のアニメーション
	void StartAnimation();
	// 最後のアニメーション
	void EndAnimation();

};