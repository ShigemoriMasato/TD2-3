#include "FontObject.h"
#include <Utility/Matrix.h>

void FontObject::Initialize(const std::string& fontName, const std::wstring& text, DrawData drawData, FontLoader* fontLoader) {
	fontName_ = fontName;
	text_ = text;

	// レンダリングオブジェクトの作成
	renderObject_ = std::make_unique<RenderObject>("FontObject");
	renderObject_->Initialize();
	// シェーダーの設定
	renderObject_->psoConfig_.vs = "Font/FontBasic.VS.hlsl";
	renderObject_->psoConfig_.ps = "Font/FontBasic.PS.hlsl";
	// バッファの作成
	renderObject_->CreateCBV(sizeof(Matrix4x4), ShaderType::VERTEX_SHADER, "FontObject::WVPMatrix");
	renderObject_->CreateSRV(sizeof(CharPosition), 256, ShaderType::VERTEX_SHADER, "FontObject::CharPosition");
	renderObject_->CreateCBV(sizeof(int), ShaderType::PIXEL_SHADER, "FontObject::TextureIndex");
	renderObject_->CreateCBV(sizeof(Vector4), ShaderType::PIXEL_SHADER, "FontObject::FontColor");

	// 描画データの設定
	renderObject_->SetDrawData(drawData);

	// テクスチャ使用フラグ
	renderObject_->SetUseTexture(true);

	// フォントの読み込み
	textureIndex_ = fontLoader->Load(fontName_);

	// 文字位置情報の予約
	charPositions_.reserve(256);

	// 初期のトランスフォーム設定
	transform_.scale = { 1.0f, -1.0f, 1.0f };
	transform_.rotate = { 0.0f, 0.0f, 0.0f };
	transform_.position = { 640.0f, 360.0f, 0.0f };

	// 描画するテキストを登録
	charPositions_.clear();
	for (const wchar_t& c : text_) {
		CharPosition charPos = fontLoader->GetCharPosition(fontName_, c, 64);
		charPositions_.push_back(charPos);
	}
}

void FontObject::Update() {
	// テキストが変更された場合のみ更新
	//if (isDirty_) {
	//	UpdateCharPositions();
	//	isDirty_ = false;
	//}

}

void FontObject::Draw(Window* window, const Matrix4x4& vpMatrix) {
	if (charPositions_.empty()) {
		return;
	}

	// ワールド行列の計算
	wvpMatrix_ = Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.position) * vpMatrix;

	// バッファにデータをコピー
	renderObject_->CopyBufferData(0, &wvpMatrix_, sizeof(Matrix4x4));
	renderObject_->CopyBufferData(1, charPositions_.data(), sizeof(CharPosition) * charPositions_.size());
	renderObject_->CopyBufferData(2, &textureIndex_, sizeof(int));
	renderObject_->CopyBufferData(3, &fontColor_, sizeof(Vector4));

	// インスタンス数を設定
	renderObject_->instanceNum_ = static_cast<uint32_t>(charPositions_.size());

	// 描画
	renderObject_->Draw(window);
}

void FontObject::UpdateCharPositions(const std::wstring& text,FontLoader* fontLoader) {
	charPositions_.clear();

	// テキストを設定
	text_ = text;

	// テキストを更新
	for (const wchar_t& c : text_) {
		CharPosition charPos = fontLoader->GetCharPosition(fontName_, c, 64);
		charPositions_.push_back(charPos);
	}
}

void FontObject::UpdateCharPositionsAndFont(const std::string& fontName, const std::wstring& text, FontLoader* fontLoader) {
	charPositions_.clear();

	// テキストを設定
	text_ = text;
	fontName_ = fontName;

	// フォントの読み込み
	textureIndex_ = fontLoader->Load(fontName_);

	// テキストを更新
	for (const wchar_t& c : text_) {
		CharPosition charPos = fontLoader->GetCharPosition(fontName_, c, 64);
		charPositions_.push_back(charPos);
	}
}