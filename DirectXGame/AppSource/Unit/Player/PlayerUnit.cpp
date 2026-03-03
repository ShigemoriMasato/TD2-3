#include"PlayerUnit.h"
#include"FpsCount.h"
#include"Utility/Easing.h"
#include <Common/DebugParam/GameParamEditor.h>

#ifdef USE_IMGUI
#include <imgui/imgui.h>
#endif

void PlayerUnit::Init(MapChipField* mapChipField, DrawData drawData, int pIndex, const Vector3& pos, KeyManager* keyManager) {
	// マップデータ
	mapChipField_ = mapChipField;

	// マップの最大サイズを取得
	mapMapSize_ = mapChipField_->GetMaxMapSize();

	// 入力を取得
	keyManager_ = keyManager;

	// オブジェクトを初期化
	object_ = std::make_unique<PlayerUnitObject>();
	object_->Initialize(drawData,pIndex);

	oreOutLineObject_ = std::make_unique<OreOutLineObject>();
	oreOutLineObject_->Initialize(drawData);

	// 初期位置を設定
	object_->transform_.position = pos;
	object_->transform_.rotate.y = std::numbers::pi_v<float>;
	dir_ = { 0,0,-1 };

	// 当たり判定の設定
	circleCollider_.center = object_->transform_.position;
	circleCollider_.radius = 1.0f;

	// 当たり判定の要素を設定
	CollConfig config;
	config.colliderInfo = &circleCollider_;
	config.isActive = true;
	config.ownTag = CollTag::Player;
	config.targetTag = static_cast<uint32_t>(CollTag::Unit);
	SetColliderConfig(config);

	// 当たり判定の初期化
	Initialize();

#ifdef USE_IMGUI
	// 値の登録
	RegisterDebugParam();
#endif
	// 値の適応
	ApplyDebugParam();
}

void PlayerUnit::Update() {
#ifdef USE_IMGUI
	// 値の適応
	ApplyDebugParam();
#endif

	if (isAutoMove_) {

		if (path_.empty()) {
			Vector3 toTarget = targetPos_ - object_->transform_.position;
			toTarget.y = 0.0f;
			// XZ平面での距離を計算
			float distance = std::sqrt(toTarget.x * toTarget.x + toTarget.z * toTarget.z);

			object_->transform_.position += (toTarget / distance) * speed_ * FpsCount::deltaTime;

			// 自動移動状態を解除する
			if (distance < 0.1f) {
				isAutoMove_ = false;
			}
		} else {
			// 自動移動
			AutoMove();
		}

		// 自動回転
		AutoRotate();
	} else {

		// 入力処理
		ProcessMoveInput();

		// 衝突判定情報を初期化
		MapChipField::CollisionMapInfo collisionMapInfo;
		// 移動量に速度の値をコピー
		collisionMapInfo.move = velocity_;
		collisionMapInfo.pos = object_->transform_.position;
		collisionMapInfo.size = size_;

		// マップ衝突判定
		CheckMapCollision(collisionMapInfo);

		// 移動
		object_->transform_.position += collisionMapInfo.move;

		// 回転処理
		Rotate();
	}

	// アニメーション処理
	AnimationUpdate();

	object_->transform_.position.x = std::clamp(object_->transform_.position.x,0.0f, mapMapSize_.x - 1.0f);
	object_->transform_.position.z = std::clamp(object_->transform_.position.z,0.0f, mapMapSize_.y - 1.0f);

	// 更新処理
	object_->Update();

	oreOutLineObject_->transform_ = object_->transform_;
	oreOutLineObject_->transform_.scale = object_->transform_.scale * 1.08f;
	oreOutLineObject_->Update();

	// 当たり判定の位置を更新
	circleCollider_.center.x = object_->transform_.position.x;
	circleCollider_.center.y = object_->transform_.position.z;
}

void PlayerUnit::Draw(Window* window, const Matrix4x4& vpMatrix) {

	oreOutLineObject_->Draw(window, vpMatrix);
	// 描画
	object_->Draw(window, vpMatrix);

//#ifdef USE_IMGUI
//	ImGui::Begin("Player");
//
//	ImGui::DragFloat3("PlayerPos", &object_->transform_.position.x);
//
//	ImGui::End();
//#endif
}

void PlayerUnit::OnCollision(Collider* other) {
	bool isUnit = CollTag::Unit == other->GetOwnTag();

	if (!isUnit) { return; }
}

void PlayerUnit::ProcessMoveInput() {

	// 速度
	velocity_ = { 0.0f,0.0f,0.0f };
	isMove_ = false;

	// 操作
	auto key = keyManager_->GetKeyStates();

	Vector3 moveDir = { 0,0,0 };

	if (key[Key::Up]) {
		velocity_.z = speed_ * FpsCount::deltaTime;
		//dir_ = { 0.0f,0.0f,1.0f };
		moveDir.z = 1.0f;
		isMove_ = true;
	}

	if (key[Key::Down]) {
		velocity_.z = -speed_ * FpsCount::deltaTime;
		//dir_ = { 0.0f,0.0f,-1.0f };
		moveDir.z = -1.0f;
		isMove_ = true;
	}

	if (key[Key::Left]) {
		velocity_.x = -speed_ * FpsCount::deltaTime;
		//dir_ = { -1.0f,0.0f,0.0f };
		moveDir.x = -1.0f;
		isMove_ = true;
	}

	if (key[Key::Right]) {
		velocity_.x = speed_ * FpsCount::deltaTime;
		//dir_ = { 1.0f,0.0f,0.0f };
		moveDir.x = 1.0f;
		isMove_ = true;
	}

	if (moveDir != Vector3(0,0,0)) {
		dir_ = moveDir;
	}

	if (!isAnimation_ && isMove_) {
		isAnimation_ = true;
	}
}

void PlayerUnit::CheckMapCollision(MapChipField::CollisionMapInfo& info) {
	// 上方向の衝突判定
	CheckCollisionAbove(info);
	// 下方向の衝突判定
	CheckCollisionBelow(info);
	// 右方向の衝突判定
	CheckCollisionRight(info);
	// 左方向の衝突判定
	CheckCollisionLeft(info);
}

void PlayerUnit::CheckCollisionAbove(MapChipField::CollisionMapInfo& info) {
	// 上移動がなければ早期リターン
	if (info.move.z <= 0.0f) {
		return;
	}

	// 上の判定を取る
	if (mapChipField_->IsBlockHit(MapChipField::MoveDir::Above, info)) {
		// めり込みを排除する方向に移動量を設定
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(object_->transform_.position + Vector3(0.0f, 0.0f, info.move.z + size_.z / 2.0f));
		// 現在の座用が壁の外かを判定
		MapChipField::IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(object_->transform_.position + Vector3(0.0f, 0.0f, +size_.z / 2.0f));
		if (indexSetNow.zIndex != indexSet.zIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.zIndex);
			info.move.z = std::max(0.0f, (rect.bottom - object_->transform_.position.z) - (size_.z / 2.0f));
			// 壁に当たったことを記録する
			info.isWallHit = true;
		}
	}
}

void PlayerUnit::CheckCollisionBelow(MapChipField::CollisionMapInfo& info) {
	// 下移動がなければ早期リターン
	if (info.move.z >= 0.0f) {
		return;
	}

	// 下の判定を取る
	if (mapChipField_->IsBlockHit(MapChipField::MoveDir::Below, info)) {
		// めり込みを排除する方向に移動量を設定
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(object_->transform_.position + Vector3(0.0f, 0.0f, info.move.z - size_.z / 2.0f));
		// 現在の座用が壁の外かを判定
		MapChipField::IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(object_->transform_.position + Vector3(0.0f, 0.0f, -size_.z / 2.0f));
		if (indexSetNow.zIndex != indexSet.zIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.zIndex);
			info.move.z = std::min(0.0f, (rect.top - object_->transform_.position.z) * -1.0f + (size_.z / 2.0f));
			// 壁に当たったことを記録する
			info.isWallHit = true;
		}
	}
}

void PlayerUnit::CheckCollisionRight(MapChipField::CollisionMapInfo& info) {
	// 右移動がなければ早期リターン
	if (info.move.x <= 0.0f) {
		return;
	}

	// 右の判定を取る
	if (mapChipField_->IsBlockHit(MapChipField::MoveDir::Right, info)) {
		// めり込みを排除する方向に移動量を設定
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(object_->transform_.position + Vector3(info.move.x + size_.x / 2.0f, 0.0f, 0.0f));
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.zIndex);
		info.move.x = std::min(0.0f, (rect.left - object_->transform_.position.x) - (size_.x / 2.0f));
		// 壁に当たったことを記録する
		info.isWallHit = true;
	}
}

void PlayerUnit::CheckCollisionLeft(MapChipField::CollisionMapInfo& info) {
	// 左移動がなければ早期リターン
	if (info.move.x >= 0.0f) {
		return;
	}

	// 左の判定を取る
	if (mapChipField_->IsBlockHit(MapChipField::MoveDir::Left, info)) {
		// めり込みを排除する方向に移動量を設定
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(object_->transform_.position + Vector3(info.move.x - size_.x / 2.0f, 0.0f, 0.0f));
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.zIndex);
		info.move.x = std::max(0.0f, (rect.right - object_->transform_.position.x) + (size_.x / 2.0f));
		// 壁に当たったことを記録する
		info.isWallHit = true;
	}
}

void PlayerUnit::Rotate() {

	Vector3 targetRot = { 0, 0, 0 };
	// Y軸回転を取得
	targetRot.y = atan2f(dir_.x, dir_.z);
	Vector3 currentRot = object_->transform_.rotate;

	// 最短距離の角度を取得
	float diffY = GetShortAngleY(targetRot.y - currentRot.y);

	// 回転
	currentRot.y += diffY * rotateSpeed_ * FpsCount::deltaTime;
	object_->transform_.rotate = currentRot;
}

void PlayerUnit::AnimationUpdate() {
	if (isAnimation_) {
		animationTimer_ += FpsCount::deltaTime / moveAnimationTime_;

		if (animationTimer_ <= 0.4f) {
			float localT = animationTimer_ / 0.4f;
			object_->transform_.position.y = lerp(animeMinHeight_, animeMaxHeight_, localT, EaseType::EaseInOutCubic);

			// スケール
			float width = lerp(1.0f, minWidth_, localT, EaseType::EaseInOutCubic);
			object_->transform_.scale.x = width;
			object_->transform_.scale.z = width;
			object_->transform_.scale.y = lerp(1.0f, maxHeight_, localT, EaseType::EaseInOutCubic);
		} else if(animationTimer_ <= 0.8f) {
			float localT = (animationTimer_ - 0.4f) / 0.4f;
			object_->transform_.position.y = lerp(animeMaxHeight_, animeMinHeight_, localT, EaseType::EaseInCubic);

			// スケール
			float width = lerp(minWidth_, maxWidth_, localT, EaseType::EaseInOutCubic);
			object_->transform_.scale.x = width;
			object_->transform_.scale.z = width;
			object_->transform_.scale.y = lerp(maxHeight_, minHeight_, localT, EaseType::EaseInOutCubic);
		} else {
			float localT = (animationTimer_ - 0.8f) / 0.2f;
			// スケール
			float width = lerp(maxWidth_, 1.0f, localT, EaseType::EaseInOutCubic);
			object_->transform_.scale.x = width;
			object_->transform_.scale.z = width;
			object_->transform_.scale.y = lerp(minHeight_, 1.0f, localT, EaseType::EaseInOutCubic);
		}

		if (animationTimer_ >= 1.0f) {
			animationTimer_ = 0.0f;
			object_->transform_.scale = { 1.0f,1.0f,1.0f };

			if (isMove_) {
				isAnimation_ = true;
			} else {
				isAnimation_ = false;
			}
		}
	}
}

void PlayerUnit::SetMovePos(const Vector3& targetPos) {
	isAutoMove_ = true;
	targetPos_ = targetPos;
	// 経路を設定
	CalculatePath(targetPos);
}

void PlayerUnit::AutoMove() {

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

void PlayerUnit::AutoRotate() {
	// 回転処理
	Vector3 nextTarget = {};
	if (!path_.empty()) {
		nextTarget = path_.front();
	}
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

void PlayerUnit::CalculatePath(const Vector3& goal) {
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

void PlayerUnit::RegisterDebugParam() {
	// 登録
	GameParamEditor::GetInstance()->AddItem(kGroupName_,"Speed", speed_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "ColliderRadius", circleCollider_.radius);

	// アニメーション
	GameParamEditor::GetInstance()->AddItem("Player_Animation", "MaxTime", moveAnimationTime_,0);
	GameParamEditor::GetInstance()->AddItem("Player_Animation", "MaxJumpHeight", animeMaxHeight_,1);
	GameParamEditor::GetInstance()->AddItem("Player_Animation", "MinPosY", animeMinHeight_, 1);
	GameParamEditor::GetInstance()->AddItem("Player_Animation", "MaxWidth", maxWidth_,2);
	GameParamEditor::GetInstance()->AddItem("Player_Animation", "MinWidth", minWidth_,3);
	GameParamEditor::GetInstance()->AddItem("Player_Animation", "MaxHeight", maxHeight_,4);
	GameParamEditor::GetInstance()->AddItem("Player_Animation", "MinHeight", minHeight_,5);
}

void PlayerUnit::ApplyDebugParam() {
	// 適応
	speed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "Speed");
	circleCollider_.radius = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "ColliderRadius");

	// アニメーション
	moveAnimationTime_ = GameParamEditor::GetInstance()->GetValue<float>("Player_Animation", "MaxTime");
	animeMaxHeight_ = GameParamEditor::GetInstance()->GetValue<float>("Player_Animation", "MaxJumpHeight");
	animeMinHeight_ = GameParamEditor::GetInstance()->GetValue<float>("Player_Animation", "MinPosY");
	maxWidth_ = GameParamEditor::GetInstance()->GetValue<float>("Player_Animation", "MaxWidth");
	minWidth_ = GameParamEditor::GetInstance()->GetValue<float>("Player_Animation", "MinWidth");
	maxHeight_ = GameParamEditor::GetInstance()->GetValue<float>("Player_Animation", "MaxHeight");
	minHeight_ = GameParamEditor::GetInstance()->GetValue<float>("Player_Animation", "MinHeight");
}