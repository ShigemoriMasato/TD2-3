#pragma once
#include <Render/RenderObject.h>
#include"Render/DrawDataManager.h"
#include"Utility/DataStructures.h"

class SpriteObject {
public:

	// 定数バッファ
	struct ConstBufferData {
		Vector4 color;
		Matrix4x4 uvMatrix;
		Matrix4x4 WVP;
		uint32_t textureIndex_ = 0;
		float padding[3];
	};

	~SpriteObject();

	// 初期化
	void Initialize(DrawData drawData,const Vector2& size, BlendStateID blend = BlendStateID::Normal);

	// 更新処理
	void Update();

	// 描画
	void Draw(Window* window, const Matrix4x4& vpMatrix);

	void SetTexture(const uint32_t& texutre) {
		texture_ = texutre;
	}

	// 親を設定
	void SetParent(Matrix4x4* parent) {
		parent_ = parent;
	}

public:

	// srt
	Transform transform_;

	// 色
	Vector4 color_ = { 1.0f,1.0f,1.0f,1.0f };

	uint32_t texture_ = 0;

	Matrix4x4* parent_ = nullptr;

private:

	// uv
	//Transform uvTransform_;

	//　オブジェクトデータ
	std::unique_ptr<RenderObject> renderObject_;

	// worldTransform関係
	ConstBufferData cBuffData_{};
	int cBuffDataIndex_ = -1;
	int pcBuffDataIndex_ = -1;
	Matrix4x4 worldMatrix_;
};