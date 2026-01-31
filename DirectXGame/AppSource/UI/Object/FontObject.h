#pragma once
#include <Render/RenderObject.h>
#include<Assets/Fonts/FontLoader.h>
#include <Camera/Camera.h>
#include"Utility/DataStructures.h"
#include <string>
#include <vector>

class FontObject {
public:
	FontObject() = default;
	~FontObject() = default;

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="fontName">フォント名</param>
	/// <param name="text">描画したい文字</param>
	void Initialize(const std::string& fontName, const std::wstring& text, DrawData drawData, FontLoader* fontLoader);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="window">描画先のウィンドウ</param>
	void Draw(Window* window,const Matrix4x4& vpMatrix);

	/// <summary>
	/// テキストを設定
	/// </summary>
	/// <param name="text">設定するテキスト</param>
	void SetText(const std::wstring& text) { text_ = text; isDirty_ = true; }

	/// <summary>
	/// 文字位置情報を更新
	/// </summary>
	void UpdateCharPositions(const std::wstring& text,FontLoader* fontLoader);

	/// <summary>
	/// 文字とフォントの更新
	/// </summary>
	/// <param name="text"></param>
	/// <param name="fontLoader"></param>
	void UpdateCharPositionsAndFont(const std::string& fontName,const std::wstring& text, FontLoader* fontLoader);

	// テキストの数
	int32_t GetTextSize() { return static_cast<int32_t>(charPositions_.size()); }

public:

	// トランスフォーム
	Transform transform_{};

	// フォントカラー
	Vector4 fontColor_ = { 1.0f, 1.0f, 1.0f, 1.0f };

private:

	// レンダリングオブジェクト
	std::unique_ptr<RenderObject> renderObject_;

	// テキスト情報
	std::wstring text_;
	std::string fontName_;
	
	// 文字位置情報
	std::vector<CharPosition> charPositions_;

	// ワールドビュープロジェクション行列
	Matrix4x4 wvpMatrix_{};

	// テクスチャインデックス
	int textureIndex_ = -1;

	// 更新フラグ
	bool isDirty_ = true;	
};