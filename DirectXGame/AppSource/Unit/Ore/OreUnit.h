#pragma once
#include <optional>
#include<array>
#include<functional>
#include<numbers>

#include"Collision/Collider.h"

#include"OreUnitObject.h"
#include"Unit/MapChipField.h"

#include"Unit/UnitMarkUIManager.h"

#include"Unit/UnitEffectManager.h"

class OreItem;

class OreUnit : public Collider {
public:

	// 状態
	enum class State {
		GoTo,   // 目的地に行く
		Mining, // 採掘
		ToDeliver, // 納品
		Return, // 帰宅

		MaxCount // 数
	};

	// 帰宅する時の動き
	enum class ReturnPhase {
		Rise,     // 上昇
		Move,     // 移動
		Fall,     // 落下
	};

public:

	OreUnit(MapChipField* mapChipField, DrawData drawData, int texture, Vector3* playerPos, UnitMarkUIManager* unitMarkUIManager, UnitEffectManager* unitEffectManager,int32_t damagePram);

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="mapChipField">マップ機能</param>
	/// <param name="drawData">描画データ</param>
	/// <param name="apearPos">出現位置</param>
	/// <param name="targetPos">目標位置</param>
	void Init(const Vector3& apearPos,const Vector3& targetPos, OreItem* oreItem,uint32_t unitGroupId);

	void Update();

	void Draw(Window* window, const Matrix4x4& vpMatrix);

	// 当たり判定
	void OnCollision(Collider* other) override;

	// oreItem側から削除を受け取る
	void NotifyOreDespawn() {
		oreItem_ = nullptr;
	}

public:

	// 死亡フラグを取得
	bool IsDead()const { return isDead_; }

	// 有効フラグ
	bool IsActive() const { return isActive_; }

	// 向いている方向を取得
	Vector3 GetDir() const { return dir_; }

	Vector3 GetPos() const { return object_->transform_.position; }

	int32_t GetHp() const { return hp_; }
	int32_t GetMaxHp() const { return maxHp_; }

	State GetState() const { return state_; }

	// 衝突判定
	bool IsConflict() const { return isConflict_; }

	// ユニットグループIdを取得
	uint32_t UnitGroupId() const { return unitGroupId_; }

private:
	// マップデータ
	MapChipField* mapChipField_ = nullptr;

	// 採取する鉱石のデータを取得
	OreItem* oreItem_ = nullptr;

	// ユニットマークUI
	UnitMarkUIManager* unitMarkUIManager_ = nullptr;

	// 演出管理
	UnitEffectManager* unitEffectManager_ = nullptr;

	// オブジェクトデータ
	std::unique_ptr<OreUnitObject> object_;

	// 状態
	State state_ = State::GoTo;
	// 状態の変更を管理
	std::optional<State> stateRequest_ = std::nullopt;
	// 状態テーブル
	std::array<std::function<void()>, static_cast<size_t>(State::MaxCount)> statesTable_;
	// 指定した状態をおこなうためのリセット処理
	std::array<std::function<void()>, static_cast<size_t>(State::MaxCount)> resetStatesTable_;

	// 死亡フラグ
	bool isDead_ = false;
	// 有効フラグ
	bool isActive_ = false;
	// 回収した
	bool isToDeliver_ = false;

	// 体力
	int32_t hp_ = 0;

	// 移動ルート
	std::vector<Vector3> path_;
	// 家の場所
	Vector3 homePos_ = {};
	// 目的の位置
	Vector3 targetPos_ = {};
	// プレイヤーの位置
	Vector3* playerPos_ = nullptr;
	// 回転する移動先
	Vector3 toRotPos_ = {};
	// 方向
	Vector3 dir_ = {};

	Vector3 startFixScale_ = {};

	// タイマー
	float timer_ = 0.0f;
	float lifeTimer_ = 0.0f;
	float animationTimer_ = 0.0f;
	float deathAnimationTimer_ = 0.0f;

	// 円の当たり判定
	Circle circleCollider_;

	// ヒットフラグ
	bool isHit = false;

	// 衝突フラグ
	bool isConflict_ = false;
	// クールタイム
	float conflictCoolTimer_ = 0.0f;
	float conflictCoolTime_ = 1.0f;
	bool isCoolTimeEnd_ = false;

	// 鉱石からの離脱を取得
	bool isRemoveOre_ = false;

	// 拠点に戻るとのフェーズ
	ReturnPhase returnPhase_ = ReturnPhase::Fall;
	Vector3 startMovePos;
	Vector3 endMovePos;

	// デバック用
	std::string kGroupName_ = "OreUnit";

	bool isDeathAnimation_ = false;
	bool isEndDeathAnimation_ = false;

	bool isSlow_ = false;
	float slowAnimaitonTimer_ = 0.0f;
	float slowAnimaitonTime_ = 1.0f;
	float speed_ = 0.0f;

	// ユニットグループId
	uint32_t unitGroupId_ = 0;

	// ダメー字パラメータ
	int32_t damageParam_ = 0;

	// 鉱石を取ったあとのクールタイム
	float coolTimer_ = 0.0f;
	bool isCoolActive_ = false;

	/// 音声

	// 納品音声
	uint32_t deliverSH_ = 0;

private: // 調整項目

	// ダメージを食らう間隔
	float damageTime_ = 1.0f;

	// 最大体力
	int32_t maxHp_ = 30;

	// 移動速度
	float moveSpeed_ = 2.0f;

	// 鈍足状態の移動速度
	float slowspeed_ = 0.5f;

	// 角度補間
	float rotateSpeed_ = 3.0f;

	// 採掘時間
	float miningTime_ = 1.0f;

	// 高さ
	float risePosY_ = 5.0f;

	// 帰宅の時間
	float riseTime_ = 1.0f;
	float moveTime_ = 1.0f;
	float FallTime_ = 1.0f;

	// 移動アニメーション
	float moveAnimationTime_ = 0.6f;

	float maxJumpHeight_ = 1.0f;
	float maxWidth_ = 1.2f;
	float minWidth_ = 0.8f;
	float maxHeight_ = 1.2f;
	float minHeight_ = 0.8f;

	// 死亡演出
	float deathAnimationTime_ = 1.5f;

	// ダメージパーセント
	float damagePer_ = 0.15f;

private:

	// 移動経路を求める
	void CalculatePath(const Vector3& goal);

	// 移動
	void Move();

	// 進行方向に回転する
	void Rotate();

	// 行きの更新処理
	void GoToUpdate();

	// 採掘の更新処理
	void MiningUpdate();

	// 納品の更新処理
	void ToDeliverUpdate();

	// 返りの更新処理
	void ReturnUpdate();

	// 移動アニメーション処理
	void MoveAnimationUpdate();

	// 死亡アニメーション
	void DeathAnimationUpdate();

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
	float GetShortAngleY(float diffY);
}