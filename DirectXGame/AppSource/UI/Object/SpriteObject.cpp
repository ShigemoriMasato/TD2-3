#include"SpriteObject.h"
#include"Utility/MatrixFactory.h"
#include<numbers>

SpriteObject::~SpriteObject() {

}

void SpriteObject::Initialize(DrawData drawData, const Vector2& size, BlendStateID blend) {

	// psoを設定
	renderObject_ = std::make_unique<RenderObject>();
	renderObject_->Initialize();
	renderObject_->psoConfig_.inputLayoutID = InputLayoutID::Default;
	renderObject_->psoConfig_.vs = "Sprite.VS.hlsl";
	renderObject_->psoConfig_.ps = "Sprite.PS.hlsl";
	renderObject_->psoConfig_.blendID = blend;

	// 描画モデルを設定
	renderObject_->SetDrawData(drawData);
	renderObject_->SetUseTexture(true);

	// worldTransformを登録
	cBuffDataIndex_ = renderObject_->CreateCBV(sizeof(ConstBufferData), ShaderType::VERTEX_SHADER, "TestScene::VSData");
	pcBuffDataIndex_ = renderObject_->CreateCBV(sizeof(ConstBufferData), ShaderType::PIXEL_SHADER, "TestScene::PSData");

	// 画像のサイズ設定
	transform_.scale.x = size.x;
	transform_.scale.y = size.y;

	// 回転して辻褄を合わせる
	transform_.rotate.z = -std::numbers::pi_v<float>;

	// uv行列の初期化
	cBuffData_.uvMatrix = Matrix4x4::Identity();

	// 更新
	Update();
}

void SpriteObject::Update() {
	// worldTransformを更新
	worldMatrix_ = Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.position);
	if (parent_) {
		worldMatrix_ *= *parent_;
	}
	cBuffData_.color = color_;
	cBuffData_.textureIndex_ = texture_;
	//cBuffData_.uvMatrix;
}

void SpriteObject::Draw(Window* window, const Matrix4x4& vpMatrix) {

	// カメラを設定
	cBuffData_.WVP = worldMatrix_ * vpMatrix;
	cBuffData_.color = color_;
	cBuffData_.textureIndex_ = texture_;
	renderObject_->CopyBufferData(cBuffDataIndex_, &cBuffData_, sizeof(ConstBufferData));
	renderObject_->CopyBufferData(pcBuffDataIndex_, &cBuffData_, sizeof(ConstBufferData));

	// 描画
	renderObject_->Draw(window);
}