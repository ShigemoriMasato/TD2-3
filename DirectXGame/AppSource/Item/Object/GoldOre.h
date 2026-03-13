#pragma once
#include"Collision/Collider.h"

#include"OreItem.h"
#include"OreObject.h"
#include <vector>
#include <algorithm>

class GoldOre : public OreItem,public Collider {
public:
	~GoldOre() override;

	void Init(DrawData drawData,int texture, const Vector3& pos, OreType type,const float& rotY);

	void Update() override;

	void Draw(Window* window, const Matrix4x4& vpMatrix) override;

	void OnCollision(Collider* other) override;

public:

	// タイプ
	OreType GetType() override { return type_; }

	// 位置
	Vector3 GetPos() override { return object_->transform_.position; }

	Vector3* GetPosPtr() override { return &object_->transform_.position; }

	float GetRotY() override { return object_->transform_.rotate.y; }

	// サイズ
	Vector3 GetSize() override { return size_; }

	// 死亡フラグ
	bool IsDead() override { return isDead_; }

	// 満員かどうか
	int32_t IsFullWorker(const int32_t& num) override {
		// 上限を設定
		int32_t effectiveLimit = std::min(maxWorkerNum_, hp_);
		return effectiveLimit - (num + currentWorkerNum_);
	}

	// 労働者を増やす
	void AddWorker() override {
		currentWorkerNum_++;
	}

	// 労働者を減らす
	void RemoveWorker() override {
		if (currentWorkerNum_ > 0) {
			currentWorkerNum_--;
		}
	}

	// hpを減らす
	void RemoveHp() {
		if (hp_ > 0) {
			hp_--;
		}
	}

	// 登録処理
	void RegisterUnit(OreUnit* unit) override {
		if (unit) {
			linkedUnits_.push_back(unit);
		}
	}

	// 解除処理
	void UnregisterUnit(OreUnit* unit) override {
		// リストから削除
		auto it = std::remove(linkedUnits_.begin(), linkedUnits_.end(), unit);
		linkedUnits_.erase(it, linkedUnits_.end());
	}

	// デバック情報
	int32_t GetCurrentWorkerNum() override { return currentWorkerNum_; }
	int32_t GetMaxWorkerNum() override { return std::min(hp_,maxWorkerNum_); }
	int32_t GetMaxHp() override { return hp_; }
	int32_t GetHp() override { return maxHp_; }
	bool IsChangeHp() override { return isChangeHp_; }

private:
	// オブジェクトデータ
	std::unique_ptr<OreObject> object_;

	// 死亡フラグ
	bool isDead_ = false;

	bool isChangeHp_ = false;

	// 体力
	int32_t hp_ = 0;

	Vector3 size_ = {3.0f,3.0f,3.0f};

	// 円の当たり判定
	Quad quadCollider_;

	// デバック用
	std::string kGroupName_ = "GoldOreItem";

	// 現在向かっている/作業中の数
	int32_t currentWorkerNum_ = 0;
	// 最大の数
	int32_t maxWorkerNum_ = 5;

	// タイプ
	OreType type_;


	int frame_ = 0;

	float coolTimer_ = 0.0f;
	float coolTime_ = 0.4f;

	bool isHit_ = false;

	// 自分を参照しているUnitのリスト
	std::vector<OreUnit*> linkedUnits_;

private: // 調整項目

	int32_t maxHp_ = 10;

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