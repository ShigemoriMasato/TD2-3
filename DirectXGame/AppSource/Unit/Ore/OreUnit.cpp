#include"OreUnit.h"
#include"FpsCount.h"
#include <Common/DebugParam/GameParamEditor.h>
#include"Utility/Easing.h"
#include"Utility/MyMath.h"
#include"Assets/Audio/AudioManager.h"

#include"Item/OreItemStorageNum.h"
#include"Item/Object/GoldOre.h"
#include"Item/Object/OreItem.h"
#include <TimeTracker.h>

OreUnit::OreUnit(MapChipField* mapChipField, DrawData drawData, int texture, Vector3* playerPos, UnitMarkUIManager* unitMarkUIManager, UnitEffectManager* unitEffectManager, int32_t damagePram) {

	// ダメージパラメータ
	damageParam_ = damagePram;

	// マップデータ
	mapChipField_ = mapChipField;
	
	unitMarkUIManager_ = unitMarkUIManager;
	unitEffectManager_ = unitEffectManager;

	// プレイヤー座標を取得
	playerPos_ = playerPos;

	// オブジェクトを初期化
	object_ = std::make_unique<OreUnitObject>();
	object_->Initialize(drawData,texture);

	// 更新状態を設定する
	statesTable_ = {
		[this]() { GoToUpdate(); },
		[this]() { MiningUpdate(); },
		[this]() {ToDeliverUpdate(); },
		[this]() { ReturnUpdate(); }
	};

	// 各状態のリセット処理を設定する
	resetStatesTable_ = {
		[this]() {},
		[this]() {},
		[this]() {
			// プレイヤーの位置までの経路を取得
			CalculatePath(*playerPos_);
		},
		[this]() {
			returnPhase_ = ReturnPhase::Rise;
			timer_ = 0.0f;
			toRotPos_ = homePos_;
			startFixScale_ = object_->transform_.scale;

			// 距離によって帰宅時間を求める
			Vector3 pos = homePos_ - object_->transform_.position;
			pos.y = 0.0f;
			float dis = pos.Length();
			moveTime_ = dis * 0.3f;
		}
	};

	// 当たり判定の設定
	circleCollider_.center = Vector2(object_->transform_.position.x, object_->transform_.position.z);
	circleCollider_.radius = 1.0f;

	// 当たり判定の要素を設定
	CollConfig config;
	config.colliderInfo = &circleCollider_;
	config.isActive = true;
	config.ownTag = CollTag::Unit;
	config.targetTag = static_cast<uint32_t>(CollTag::Player) | static_cast<uint32_t>(CollTag::Stage) | static_cast<uint32_t>(CollTag::Unit);
	SetColliderConfig(config);

	// 当たり判定の初期化
	Initialize();

	// 音声を取得
	deliverSH_ = AudioManager::GetInstance().GetHandleByName("GetOreItem.mp3");

	// 値の登録
	RegisterDebugParam();
	// 値の適応
	ApplyDebugParam();
}

void OreUnit::Init(const Vector3& apearPos, const Vector3& targetPos, OreItem* oreItem, uint32_t unitGroupId) {

	// ユニットIdを取得
	unitGroupId_ = unitGroupId;

	// 取得する鉱石
	oreItem_ = oreItem;

	if (oreItem_) {
		oreItem_->RegisterUnit(this);
	}

	// 初期位置を設定
	object_->transform_.position = apearPos;
	object_->transform_.scale = { 0.5f,0.5f,0.5f };
	object_->transform_.rotate.z = 0.0f;
	object_->material_.color = { 1.0f,1.0f,1.0f,1.0f };
	// 拠点位置を設定
	homePos_ = apearPos;

	// フラグをリセット
	isActive_ = true;
	isDead_ = false;
	isRemoveOre_ = false;
	isToDeliver_ = false;
	isDeathAnimation_ = false;
	isEndDeathAnimation_ = false;
	isConflict_ = false;

	// 最初は衝突判定を無効にする
	isCoolTimeEnd_ = true;
	conflictCoolTimer_ = 0.0f;

	// hpを設定
	hp_ = maxHp_;

	// 速度
	speed_ = moveSpeed_;

	// 時間をリセット
	lifeTimer_ = 0.0f;
	animationTimer_ = 0.0f;
	deathAnimationTimer_ = 0.0f;

	// 状態を設定
	state_ = State::GoTo;
	// 状態のリセット
	resetStatesTable_[static_cast<size_t>(state_)]();

	// 目的位置を取得
	targetPos_ = targetPos;

	// 移動する経路を求める
	CalculatePath(targetPos);
}

void OreUnit::Update() {
#ifdef USE_IMGUI
	// 値の適応
	ApplyDebugParam();
#endif

	if (isDead_) { return; }

	isHit = false;

	// 切り替え処理
	if (stateRequest_) {
		// 状態を変更
		state_ = stateRequest_.value();
		// 状態によるリセットを呼び出す
		resetStatesTable_[static_cast<size_t>(state_)]();
		// リクエストをクリア
		stateRequest_ = std::nullopt;
	}

	if (!isConflict_ && !isDeathAnimation_) {
		// プレイヤーの状態による更新処理をおこなう
		statesTable_[static_cast<size_t>(state_)]();
	}

	if (state_ != State::Return && !isDeathAnimation_ && !isConflict_) {
		// アニメーション処理
		MoveAnimationUpdate();
	}

	// 更新処理
	object_->Update();

	// 当たり判定の位置を更新
	circleCollider_.center = Vector2(object_->transform_.position.x, object_->transform_.position.z);

	// 当たり判定
	if (isCoolTimeEnd_) {
		conflictCoolTimer_ += FpsCount::deltaTime / conflictCoolTime_;

		if (conflictCoolTimer_ >= 1.0f) {
			conflictCoolTimer_ = 0.0f;
			isCoolTimeEnd_ = false;
		}
	}

	// 衝突した時は色を変える
	if (isConflict_) {
		if (object_->material_.color.x != 0.5f) {
			object_->material_.color = { 0.5f,0.5f,0.5f,1.0f };
			object_->transform_.position.y = 0.0f;
			object_->transform_.scale = { 1.0f,1.0f,1.0f };
			animationTimer_ = 0.0f;
		}

		// 衝突位置を設定
		unitMarkUIManager_->AddConflict(object_->transform_.position);

	} else {
		object_->material_.color = { 1.0f,1.0f,1.0f,1.0f };
	}

	// slow状態を確認
	MapChipField::IndexSet i = mapChipField_->GetMapChipIndexSetByPosition(object_->transform_.position);
	TileType type = mapChipField_->GetBlockTypeByIndex(i.xIndex,i.zIndex);
	if (type == TileType::Slow) {
		isSlow_ = true;
		speed_ = slowspeed_;

		slowAnimaitonTimer_ += FpsCount::deltaTime / slowAnimaitonTime_;
		if (slowAnimaitonTimer_ >= 1.0f) {
			slowAnimaitonTimer_ = 0.0f;

			// 泥の演出を追加する
			unitEffectManager_->AddDirt(object_->transform_.position);
		}
	} else {
		isSlow_ = false;
		speed_ = moveSpeed_;
	}

	isSlow_ = false;

	// 帰宅していれば体力処理を飛ばす
	if (state_ == State::Return) { return; }

	// 生存時間
	lifeTimer_ += FpsCount::deltaTime / damageTime_;

	if (lifeTimer_ >= 1.0f) {
		hp_ -= 1;
		lifeTimer_ = 0.0f;
	}

	// 体力が0の時、死亡する
	if (hp_ <= 0) {
		if (!isDeathAnimation_) {
			isDeathAnimation_ = true;
			unitEffectManager_->AddSmoke(object_->transform_.position);
		}
		object_->material_.color = { 0.8f,0.0f,0.0f,1.0f };
	}

	if (isDeathAnimation_) {

		// 死亡アニメーション
		DeathAnimationUpdate();

		if (isEndDeathAnimation_) {
			if (!isDead_) {
				// 登録されている労働者を減らす
				if (oreItem_) {
					oreItem_->RemoveWorker();
					oreItem_->UnregisterUnit(this); // 登録解除
					oreItem_ = nullptr;
				}
			}
			isDead_ = true;
		}
	}

	// 鉱石を撮った後のクールタイム
	if (isCoolActive_) {
		coolTimer_ += FpsCount::deltaTime;

		if (coolTimer_ >= 1.0f) {
			isCoolActive_ = false;
			coolTimer_ = 0.0f;
		}
	}
}

void OreUnit::Draw(Window* window, const Matrix4x4& vpMatrix) {
	if (isDead_) { return; }

	// 描画
	object_->Draw(window, vpMatrix);
}

void OreUnit::OnCollision(Collider* other) {

	if (isDead_) { return; }
	if (!isActive_) { return; }
	isHit = true;

	bool isStage = CollTag::Stage == other->GetOwnTag();
	bool isPlayer = CollTag::Player == other->GetOwnTag();
	bool isUnit = CollTag::Unit == other->GetOwnTag();

	// プレイヤーとの当たり判定
	if (isPlayer) {

		if (isConflict_) {
			isConflict_ = false;
			isCoolTimeEnd_ = true;
		}

		if (state_ != State::ToDeliver) { return; }
		// プレイヤーに触れれば帰宅する
		if (isActive_ && !isToDeliver_) {
			// 鉱石を収納する
			if (TimeLimit::totalSeconds > 0) {
				OreItemStorageNum::currentOreItemNum_ += 1;

				// 回収音声
				AudioManager::GetInstance().Play(deliverSH_, 0.5f, false);
			}

			isToDeliver_ = true;
			// 納品したら帰宅する
			stateRequest_ = State::Return;
		}
	}

	// 鉱石との当たり判定
	if (isStage) {

		if (state_ != State::GoTo && state_ != State::Mining) { return; }

		GoldOre* goldOre = dynamic_cast<GoldOre*>(other);
		if (goldOre) {

			if (state_ == State::GoTo) {
				// 指定した鉱石に近ければ採掘に移る
				if (path_.size() > 2) { return; }

				// 採掘状態に切り替える
				stateRequest_ = State::Mining;
				//goldOre->GetContactNum();
				timer_ = 0.0f;
			} else if (state_ == State::Mining) {

				if (timer_ >= 0.8f) {

					if (!isRemoveOre_) {
						isRemoveOre_ = true;
						goldOre->RemoveWorker();
						goldOre->RemoveHp();
					}
				}
			} 
		}
	}

	// ユニット
	if (isUnit) {
		if (state_ != State::GoTo && state_ != State::ToDeliver) { return; }
		if (isCoolTimeEnd_ || isCoolActive_) { return; }

		OreUnit* oreUnit = dynamic_cast<OreUnit*>(other);

		if (oreUnit) {

			// 既に衝突していたら早期リターン
			if (oreUnit->UnitGroupId() == unitGroupId_) { return; }
			if (oreUnit->GetState() == State::Mining || oreUnit->GetState() == State::Return) { return; }

			// 内積を取得する
			float dot = MyMath::dot(oreUnit->GetDir(),dir_);

			// 衝突
			if (dot < -0.95f) {
				// 衝突処理
				if (!isConflict_) {
					isConflict_ = true;
					unitEffectManager_->AddConflict(object_->transform_.position);

					// ダメージを与える
					if (damageParam_ > 0) {
						int32_t per = static_cast<int32_t>(static_cast<float>(maxHp_) * (damagePer_ * static_cast<float>(damageParam_)));
						hp_ -= per;
					}
				}
			}
		}
	}
}

void OreUnit::GoToUpdate() {

	// 目的地付けば次の状態に切り替える
	if (path_.empty()) {

		Vector3 toTarget = targetPos_ - object_->transform_.position;
		toTarget.y = 0.0f;
		// XZ平面での距離を計算
		float distance = std::sqrt(toTarget.x * toTarget.x + toTarget.z * toTarget.z);

		object_->transform_.position += (toTarget / distance) * speed_ * FpsCount::deltaTime;

		// 目的地に到着して鉱石が存在していなかった場合、帰宅する
		if (distance < 0.1f) {
			stateRequest_ = State::ToDeliver;
		}

	} else {
		// 鉱石まで移動する
		Move();
	}

	// 進行方向に回転
	Rotate();
}

void OreUnit::MiningUpdate() {

	timer_ += FpsCount::deltaTime / miningTime_;

	if (timer_ >= 1.0f) {
		timer_ = 0.0f;
		stateRequest_ = State::ToDeliver;
		isCoolActive_ = true;
	}
}

void OreUnit::ToDeliverUpdate() {
	// 回収が終われば帰宅する
	if (path_.empty()) {
		//stateRequest_ = State::Return;
	}

	// プレイヤーが現在いるマップチップのインデックスと座標を取得
	MapChipField::IndexSet pIdx = mapChipField_->GetMapChipIndexSetByPosition(*playerPos_);
	Vector3 playerGridPos = mapChipField_->GetMapChipPositionByIndex(pIdx.xIndex, pIdx.zIndex);

	// 「現在の目的地（パスの終点）」と「プレイヤーのいるマスの中心」が違うかチェック
	bool needRecalc = false;
	if (!path_.empty()) {
		Vector3 currentDest = path_.back(); // 現在のパスの最終地点

		// 距離の二乗で比較
		float dx = currentDest.x - playerGridPos.x;
		float dz = currentDest.z - playerGridPos.z;
		if ((dx * dx + dz * dz) > 0.1f) {
			// 目的地が変わっているので再計算フラグを立てる
			needRecalc = true;
		}
	}

	// 再計算が必要なら実行
	if (needRecalc) {
		CalculatePath(*playerPos_);
	}

	// 移動する
	Move();

	// 進行方向に回転
	Rotate();
}

void OreUnit::ReturnUpdate() {

	switch (returnPhase_)
	{
	case ReturnPhase::Rise:

		timer_ += FpsCount::deltaTime / riseTime_;

		// 移動
		object_->transform_.position.y = lerp(0.0f, risePosY_, timer_, EaseType::EaseInOutQuad);

		// 回転移動
		Rotate();

		// スケールが正常で無ければ元に戻す
		if (startFixScale_ != Vector3(1.0f,1.0f,1.0f)) {
			object_->transform_.scale = lerp(startFixScale_, { 1.0f,1.0f,1.0f }, timer_, EaseType::EaseInOutCubic);
		}
		
		// 終了
		if (timer_ >= 1.0f) {
			object_->transform_.position.y = risePosY_;
			timer_ = 0.0f;

			// 拠点までのベクトルを求める
			Vector3 toHome = homePos_ - object_->transform_.position;
			toHome.y = 0.0f;
			toHome.Normalize();

			// 移動する位置を設定
			startMovePos = object_->transform_.position;
			endMovePos = Vector3(homePos_.x, risePosY_, homePos_.z);

			returnPhase_ = ReturnPhase::Move;
		}
		break;

	case ReturnPhase::Move:

		timer_ += FpsCount::deltaTime / moveTime_;

		// 移動
		object_->transform_.position = lerp(startMovePos, endMovePos, timer_, EaseType::EaseInOutQuad);

		// 終了
		if (timer_ >= 1.0f) {
			object_->transform_.position = endMovePos;
			timer_ = 0.0f;

			returnPhase_ = ReturnPhase::Fall;
		}
		break;

	case ReturnPhase::Fall:

		timer_ += FpsCount::deltaTime / FallTime_;

		// 移動
		object_->transform_.position.y = lerp(risePosY_, 0.0f, timer_, EaseType::EaseInOutQuad);

		// 終了
		if (timer_ >= 1.0f) {
			timer_ = 0.0f;
			isActive_ = false;

			if (oreItem_) {
				oreItem_->UnregisterUnit(this); // 登録解除
				oreItem_ = nullptr;
			}
		}
		break;
	}
}

void OreUnit::CalculatePath(const Vector3& goal) {
	// メンバ変数のパスをクリア
	path_.clear();

	if (mapChipField_) {
		// マップクラスに経路計算を依頼
		// startPos は現在の自分の位置
		std::vector<Vector3> calculatedPath = mapChipField_->CalculatePath(object_->transform_.position, goal);
		path_ = calculatedPath;

		if (!path_.empty()) {
			Vector3 toFirst = path_.front() - object_->transform_.position;
			toFirst.y = 0.0f;

			// Move()で使用している判定距離(0.1f)よりも少し広めに判定をとる（例: 0.5f）
			// これにより「今のマスの中心に戻る」という挙動を防ぐ
			if (toFirst.Length() < 0.5f) {
				path_.erase(path_.begin());
			}
		}
	}

	// もし経路が見つからなかった場合
	if (path_.empty()) {
		// とりあえずゴール地点だけ入れておく
		path_.push_back(goal);
	}
}

void OreUnit::Move() {

	// パスに何も存在しなければ早期リターン
	if (path_.empty()) { return; }

	// 次に進むべき座標を取得
	Vector3 nextTarget = path_.front();
	Vector3 currentPos = object_->transform_.position;

	// 次の地点へのベクトルを計算
	Vector3 toTarget = nextTarget - currentPos;
	toTarget.y = 0.0f;

	// XZ平面での距離を計算
	float distance = std::sqrt(toTarget.x * toTarget.x + toTarget.z * toTarget.z);

	// 移動処理
	if (distance < 0.1f) {
		// 十分近づいたら座標を確定させて、リストから削除（次の地点へ）
		object_->transform_.position = nextTarget;
		path_.erase(path_.begin());
	} else {
		// 目的地に向かって移動


		Vector3 velocity = (toTarget / distance) * speed_;
		object_->transform_.position += velocity * FpsCount::deltaTime;
	}
}

void OreUnit::Rotate() {
	// 回転処理
	if (!path_.empty()) {
		toRotPos_ = path_.front();
	}
	Vector3 nextTarget = toRotPos_;
	Vector3 toTarget = nextTarget - object_->transform_.position;
	toTarget.y = 0.0f;
	toTarget.Normalize();

	// 現在の進行方向ベクトルを保持
	dir_ = toTarget;

	Vector3 targetRot = { 0, 0, 0 };
	// Y軸回転を取得
	targetRot.y = atan2f(toTarget.x, toTarget.z);
	Vector3 currentRot = object_->transform_.rotate;

	// 最短距離の角度を取得
	float diffY = GetShortAngleY(targetRot.y - currentRot.y);

	// 回転
	currentRot.y += diffY * rotateSpeed_ * FpsCount::deltaTime;
	object_->transform_.rotate = currentRot;
}

void OreUnit::MoveAnimationUpdate() {

	animationTimer_ += FpsCount::deltaTime / moveAnimationTime_;

	if (animationTimer_ <= 0.5f) {
		float localT = animationTimer_ / 0.5f;
		object_->transform_.position.y = lerp(0.0f, maxJumpHeight_, localT, EaseType::EaseInOutCubic);

		// スケール
		float width = lerp(maxWidth_, minWidth_, localT, EaseType::EaseInOutCubic);
		object_->transform_.scale.x = width;
		object_->transform_.scale.z = width;
		object_->transform_.scale.y = lerp(minHeight_, maxHeight_, localT, EaseType::EaseInOutCubic);
	} else {
		float localT = (animationTimer_ - 0.5f) / 0.5f;
		object_->transform_.position.y = lerp(maxJumpHeight_, 0.0f, localT, EaseType::EaseInCubic);
		// スケール
		float width = lerp(minWidth_, maxWidth_, localT, EaseType::EaseInOutCubic);
		object_->transform_.scale.x = width;
		object_->transform_.scale.z = width;
		object_->transform_.scale.y = lerp(maxHeight_, minHeight_, localT, EaseType::EaseInOutCubic);
	}

	if (animationTimer_ >= 1.0f) {
		animationTimer_ = 0.0f;
	}
}

void OreUnit::DeathAnimationUpdate() {

	deathAnimationTimer_ += FpsCount::deltaTime / deathAnimationTime_;

	object_->transform_.rotate.z = lerp(0.0f, std::numbers::pi_v<float> / 2.0f, deathAnimationTimer_, EaseType::EaseInOutCubic);

	if (deathAnimationTimer_ >= 1.0f) {
		object_->transform_.rotate.z = std::numbers::pi_v<float> / 2.0f;
		isEndDeathAnimation_ = true;
	}
}

void OreUnit::RegisterDebugParam() {
	// 登録
	int i = 0;
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "MoveSpeed", moveSpeed_,i++);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "SlowSpeed", slowspeed_, i++);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "RotateSpeed", rotateSpeed_, i++);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "MaxHp", maxHp_, i++);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "RiseHeight", risePosY_, i++);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "RiseReturnTime", riseTime_, i++);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "FallReturnTime", FallTime_, i++);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "MiningTime", miningTime_, i++);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "DamagePer", damagePer_, i++);

	// アニメーション
	GameParamEditor::GetInstance()->AddItem("OreUnit_Animation", "MaxJumpHeight", maxJumpHeight_,0);
	GameParamEditor::GetInstance()->AddItem("OreUnit_Animation", "MaxWidth", maxWidth_, 1);
	GameParamEditor::GetInstance()->AddItem("OreUnit_Animation", "MinWidth", minWidth_, 2);
	GameParamEditor::GetInstance()->AddItem("OreUnit_Animation", "MaxHeight", maxHeight_, 3);
	GameParamEditor::GetInstance()->AddItem("OreUnit_Animation", "MinHeight", minHeight_, 4);

	GameParamEditor::GetInstance()->AddItem("OreUnit_Animation", "DeathAnimationTime", deathAnimationTime_, 5);
}

void OreUnit::ApplyDebugParam() {
	// 適応
	moveSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "MoveSpeed");
	slowspeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "SlowSpeed");
	rotateSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "RotateSpeed");
	maxHp_ = GameParamEditor::GetInstance()->GetValue<int32_t>(kGroupName_, "MaxHp");
	risePosY_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "RiseHeight");
	riseTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "RiseReturnTime");
	FallTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "FallReturnTime");
	miningTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "MiningTime");
	damagePer_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "DamagePer");

	// アニメーション
	maxJumpHeight_ = GameParamEditor::GetInstance()->GetValue<float>("OreUnit_Animation", "MaxJumpHeight");
	maxWidth_ = GameParamEditor::GetInstance()->GetValue<float>("OreUnit_Animation", "MaxWidth");
	minWidth_ = GameParamEditor::GetInstance()->GetValue<float>("OreUnit_Animation", "MinWidth");
	maxHeight_ = GameParamEditor::GetInstance()->GetValue<float>("OreUnit_Animation", "MaxHeight");
	minHeight_ = GameParamEditor::GetInstance()->GetValue<float>("OreUnit_Animation", "MinHeight");

	deathAnimationTime_ = GameParamEditor::GetInstance()->GetValue<float>("OreUnit_Animation", "DeathAnimationTime");
}

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