#pragma once
#include<numbers>

#include"Unit/MapChipField.h"
#include"PlayerUnitObject.h"
#include"Common/KeyConfig/KeyManager.h"

#include"Collision/Collider.h"
#include"Item/Object/OreOutLineObject.h"

class PlayerUnit : public Collider {
public:

	void Init(MapChipField* mapChipField, DrawData drawData,int pIndex,const Vector3& pos, KeyManager* keyManager);

	void Update();

	void Draw(Window* window, const Matrix4x4& vpMatrix);

	// 当たり判定
	void OnCollision(Collider* other) override;

	// 移動する位置を設定する
	void SetMovePos(const Vector3& targetPos);

	// 長押しの移動
	void SetPressMove(const Vector3& pos);

public:

	Vector3* GetPos() { return &object_->transform_.position; }

private:

	// マップデータ
	MapChipField* mapChipField_ = nullptr;

	// オブジェクトデータ
	std::unique_ptr<PlayerUnitObject> object_;

	// 選択中のオブジェクトにアウトラインを描画する
	std::unique_ptr<OreOutLineObject> oreOutLineObject_;

	// キーを取得する
	KeyManager* keyManager_ = nullptr;

	// 体力
	int32_t hp_ = 0;

	// サイズ
	Vector3 size_ = { 0.8f,0.8f,0.8f };
	
	// 速度 
	Vector3 velocity_ = {};
	// 方向
	Vector3 dir_ = {};

	// 円の当たり判定
	Circle circleCollider_;

	// デバック用
	std::string kGroupName_ = "Player";

	bool isAnimation_ = false;
	float animationTimer_ = 0.0f;
	float moveAnimationTime_ = 0.2f;
	bool isMove_ = false;

	Vector2 mapMapSize_;

	// 移動ルート
	std::vector<Vector3> path_;

	bool isAutoMove_ = false;
	bool isPress_ = false;

	Vector3 targetPos_ = {};
	
private: // 調整項目

	// 速さ
	float speed_ = 10.0f;

	// 角度補間
	float rotateSpeed_ = 7.0f;

	// アニメーション
	float animeMaxHeight_ = 1.0f;
	float animeMinHeight_ = 1.0f;
	float maxWidth_ = 1.2f;
	float minWidth_ = 0.8f;
	float maxHeight_ = 1.2f;
	float minHeight_ = 0.8f;
private:

	// 入力処理
	void ProcessMoveInput();

	/// <summary>
	/// マップの衝突判定
	/// </summary>
	/// <param name="info"></param>
	void CheckMapCollision(MapChipField::CollisionMapInfo& info);

	/// <summary>
	/// 上方向の衝突判定
	/// </summary>
	/// <param name="info"></param>
	void CheckCollisionAbove(MapChipField::CollisionMapInfo& info);

	/// <summary>
	/// 下方向の衝突判定
	/// </summary>
	/// <param name="info"></param>
	void CheckCollisionBelow(MapChipField::CollisionMapInfo& info);

	/// <summary>
	/// 右方向の衝突判定
	/// </summary>
	/// <param name="info"></param>
	void CheckCollisionRight(MapChipField::CollisionMapInfo& info);

	/// <summary>
	/// 左方向の衝突判定
	/// </summary>
	/// <param name="info"></param>
	void CheckCollisionLeft(MapChipField::CollisionMapInfo& info);

	// 回転処理
	void Rotate();

	// アニメーション
	void AnimationUpdate();

	// 自動移動
	void AutoMove();

	// 自動回転
	void AutoRotate();

	// 移動経路を求める
	void CalculatePath(const Vector3& goal);

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

// ヘルプ関数
namespace {

	// 最短角度を求める
	float GetShortAngleY(float diffY) {
		while (diffY > std::numbers::pi_v<float>) {
			diffY -= std::numbers::pi_v<float> *2.0f;
		}
		while (diffY < -std::numbers::pi_v<float>) {
			diffY += std::numbers::pi_v<float> *2.0f;
		}
		return diffY;
	}
}