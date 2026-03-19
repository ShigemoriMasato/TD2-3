#include"OreItemManager.h"
#include"FpsCount.h"
#include"RandomGenerator.h"
#include"Utility/Easing.h"
#include"Assets/Audio/AudioManager.h"

// 各鉱石
#include"Object/GoldOre.h"
#include <LightManager.h>

void OreItemManager::Initialize(DrawData spriteDrawData, DrawData hpDrawData, const std::string& fontName, DrawData fontDrawData, FontLoader* fontLoader, DrawData drawData, int stageNum, int icon) {

	fontLoader_ = fontLoader;
	fontName_ = fontName;
	fontDrawData_ = fontDrawData;
	spriteDrawData_ = spriteDrawData;
	hpDrawData_ = hpDrawData;

	iconTex_ = icon;

	// メモリを確保
	oreItems_.reserve(20);
	fontList_.reserve(20);

	// アウトライン描画用
	oreOutLineObject_ = std::make_unique<OreOutLineObject>();
	oreOutLineObject_->Initialize(smallDrawData_);

	// 破片パーティクルの初期化
	oreFragmentParticle_ = std::make_unique<OreFragmentParticle>();
	oreFragmentParticle_->Initialize(drawData);

	// 鉱石の破壊演出を初期化
	oreBreakParticle_ = std::make_unique<OreBreakParticle>();
	oreBreakParticle_->Initialize(drawData);

	// 泥のパーティクル
	dirtMoveParticle_ = std::make_unique<DirtMoveParticle>();
	dirtMoveParticle_->Initialize(drawData);

	addTimeSH_ = AudioManager::GetInstance().GetHandleByName("AddTime.mp3");

	if (stageNum >= 1) {
		isLargeScale_ = true;
	}
}

void OreItemManager::SetDirtParticleEmit() {

	// 泥のある位置を取得
	std::vector<Vector3> dirtposList = MapChipField_->GetDirtPosList();

	// 位置を設定
	for (size_t i = 0; i < dirtposList.size(); ++i) {
		DirtData data;
		data.pos = dirtposList[i];
		data.timer_ = 0.0f;
		data.maxTime_ = RandomGenerator::Get(1.0f,2.0f);
		dirtDataList_.push_back(data);
	}
}

void OreItemManager::Update(bool isOpenMap) {

	// 鉱石を削除
	for (auto it = graveyard_.begin(); it != graveyard_.end(); ) {
		// カウントダウン
		it->second--;
		int lightIndex = it->first->GetLightIndex();
		auto& light = LightManager::GetInstance()->GetPointLight(lightIndex);
		light.intensity = 3.5f / (float)(it->second);

		// 待機時間が終わったら削除
		if (it->second <= 0) {
			// ライトを削除
			LightManager::GetInstance()->RemovePointLight(lightIndex);
			it = graveyard_.erase(it);
		} else {
			++it;
		}
	}

	// 鉱石の更新処理
	for (auto it = oreItems_.begin(); it != oreItems_.end(); ) {
		auto& ore = it->second;

		// 更新処理
		ore->Update();

		// 破片パーティクルを追加
		if (ore->IsChangeHp()) {
			oreFragmentParticle_->AddParticle(ore->GetPos());
		}

		// フォントを更新する
		std::wstring s = std::to_wstring(ore->GetCurrentWorkerNum()) + L"/" + std::to_wstring(ore->GetMaxWorkerNum());
		fontList_[it->first].font->UpdateCharPositions(s, fontLoader_);

		// hp処理
		float p = static_cast<float>(ore->GetMaxHp()) / static_cast<float>(ore->GetHp());
		if (isOpenMap) {

			if (isLargeScale_) {
				fontList_[it->first].hpSprite->transform_.scale.x = 5.0f * p;
				fontList_[it->first].font->transform_.scale = Vector3(0.01f, -0.01f, 1.0f) * 3.0f;
				fontList_[it->first].font->transform_.position = fontList_[it->first].fontPos + Vector3(-1.0f, 0.0f, -0.5f);
			} else {
				fontList_[it->first].font->transform_.position = fontList_[it->first].fontPos + Vector3(0.0f, 0.0f, -0.25f);
			}
		} else {
			fontList_[it->first].hpSprite->transform_.scale.x = 2.0f * p;
			fontList_[it->first].font->transform_.scale = Vector3(0.01f, -0.01f, 1.0f);
			fontList_[it->first].font->transform_.position = fontList_[it->first].fontPos + Vector3(0.0f, 0.1f, -0.1f);
		}

		float posZ = 0.0f;
		Vector3 hpPosZ = { 0,0,0 };
		if (isOpenMap) {
			posZ = fontList_[it->first].bgSprite->transform_.position.z;
			fontList_[it->first].bgSprite->transform_.position.z -= 0.2f;
			fontList_[it->first].bgSprite->transform_.rotate.x = -1.2f;
			fontList_[it->first].font->transform_.rotate.x = -1.3f;

			hpPosZ = fontList_[it->first].hpSprite->transform_.position;

			if (isLargeScale_) {
				// bg
				fontList_[it->first].bgSprite->transform_.scale *= 3.0f;
				// hp
				fontList_[it->first].hpSprite->transform_.position.y += 1.0f;
				fontList_[it->first].hpSprite->transform_.position.z -= 1.0f;
				fontList_[it->first].hpSprite->transform_.position.x += -1.5f;
				fontList_[it->first].hpSprite->transform_.rotate.x = -1.2f;
			} else {
				fontList_[it->first].hpSprite->transform_.position.y += 0.5f;
				fontList_[it->first].hpSprite->transform_.position.z -= 0.15f;
			}
		} else {
			fontList_[it->first].bgSprite->transform_.rotate.x = -2.4f;
			fontList_[it->first].font->transform_.rotate.x = -2.4f;
			fontList_[it->first].hpSprite->transform_.rotate.x = -2.4f;
		}

		// 満員なら文字の色を変える
		if (ore->GetCurrentWorkerNum() >= ore->GetMaxWorkerNum()) {
			fontList_[it->first].font->fontColor_ = { 0.8f,0.0f,0.0f,1.0f };
		} else {
			fontList_[it->first].font->fontColor_ = { 0.0f,1.0f,1.0f,1.0f };
		}

		// 更新処理
		fontList_[it->first].font->Update();
		fontList_[it->first].bgSprite->Update();
		fontList_[it->first].hpSprite->Update();

		if (isOpenMap) {
			fontList_[it->first].bgSprite->transform_.position.z = posZ;
			fontList_[it->first].hpSprite->transform_.position = hpPosZ;

			if (isLargeScale_) {
				fontList_[it->first].bgSprite->transform_.scale = { 2.0f,1.0f,1.0f };
			}
		}

		if (ore->IsDead()) {

			// 壊れた場所のマップチップ番号を空気に変換する
			Vector3 pos = ore->GetPos();
			MapChipField::IndexSet index = MapChipField_->GetMapChipIndexSetByPosition(pos);
			MapChipField_->SetMapChipBlockType(index.xIndex, index.zIndex, TileType::Air);

			// 鉱石が壊れた時の時間を追加する
			AddTimerUI(pos,ore->GetType());

			// 破壊演出を起動
			oreBreakParticle_->AddParticle(ore->GetPos());

			// 5フレーム後に削除するようにリストに追加
			graveyard_.push_back({ std::move(ore), 5 });

			// マップからは要素を削除
			it = oreItems_.erase(it);
		} else {
			it++;
		}
	}

	// 選択されていればアウトラインを更新
	if (selectId_ != -1 && oreItems_.size() > 0) {

		auto it = oreItems_.find(selectId_);
		if (it == oreItems_.end()) {
			return;
		}
		// 選択した鉱石の位置を取得
		OreItem* ore = GetOreItemForId();
		oreOutLineObject_->transform_.position = ore->GetPos();
		oreOutLineObject_->transform_.rotate.y = ore->GetRotY();
		oreOutLineObject_->Update();

		switch (ore->GetType())
		{
		case OreType::Large:
			oreOutLineObject_->SetDrawData(largeDrawData_);
			break;

		case OreType::Medium:
			oreOutLineObject_->SetDrawData(midleDrawData_);
			break;

		case OreType::Small:
			oreOutLineObject_->SetDrawData(smallDrawData_);
			break;
		}
	}


	// 演出の更新処理
	oreFragmentParticle_->Update();

	// 演出の更新
	oreBreakParticle_->Update();

	// 時間追加のUI更新処理
	TimerUIUpdate();

	// 泥の演出を追加
	if (!dirtDataList_.empty()) {

		// 位置を追加
		for (auto& dirtPos : dirtDataList_) {

			dirtPos.timer_ += FpsCount::deltaTime / dirtPos.maxTime_;

			if (dirtPos.timer_ >= 1.0f) {
				dirtPos.timer_ = 0.0f;
				dirtMoveParticle_->AddParticle(dirtPos.pos);
			}
		}

		// 更新処理
		dirtMoveParticle_->Update();
	}
}

void OreItemManager::Draw(Window* window, const Matrix4x4& vpMatrix) {

	// 鉱石を描画
	for (auto& [id, ore] : oreItems_) {
		ore->Draw(window, vpMatrix);

		// フォントを描画
		fontList_[id].bgSprite->Draw(window, vpMatrix);
		fontList_[id].hpSprite->Draw(window, vpMatrix);
		fontList_[id].font->Draw(window, vpMatrix);
	}

	// アウトラインを描画
	if (selectId_ != -1 && oreItems_.size() > 0) {
		oreOutLineObject_->Draw(window, vpMatrix);
	}

	// 泥のパーティクル
	if (!dirtDataList_.empty()) {
		dirtMoveParticle_->Draw(window, vpMatrix);
	}

	// 時間の3DUIを描画する
	for (auto& data : addTimerFontList_) {
		if (data.isFinished_) { continue; }
		data.font3d->Draw(window, vpMatrix);
		data.icon3d->Draw(window, vpMatrix);
	}
}

void OreItemManager::DrawEffect(Window* window, const Matrix4x4& vpMatrix) {
	// 演出を描画
	oreFragmentParticle_->Draw(window, vpMatrix);

	// 鉱石の破壊表現を描画
	oreBreakParticle_->Draw(window, vpMatrix);
}

void OreItemManager::DrawTimeUI(Window* window, const Matrix4x4& vpMatrix) {

	// 時間の2DUIを描画する
	for (auto& data : addTimerFontList_) {
		if (data.isFinished_) { continue; }
		data.font2d->Draw(window, vpMatrix);
		data.icon2d->Draw(window, vpMatrix);
	}
}

void OreItemManager::DrawUI() {
#ifdef USE_IMGUI

	ImGui::Begin("OreItemInfo");
	// リストの情報を表示
	ImGui::Text("Total OreItems: %d", static_cast<int>(oreItems_.size()));
	ImGui::Separator();

	for (auto& [id, ore] : oreItems_) {
		ImGui::PushID(id);

		if (ImGui::TreeNode("OreNode", "Ore ID: %d", id)) {
			// HP
			int hp = ore->GetHp();
			int maxHp = ore->GetMaxHp();
			ImGui::Text("Hp: %d / %d", hp, maxHp);

			// 労働者数
			int currentWorker = ore->GetCurrentWorkerNum();
			int maxWorker = ore->GetMaxWorkerNum();
			ImGui::Text("Workers: %d / %d", currentWorker, maxWorker);

			// 座標
			Vector3 pos = ore->GetPos();
			float p[3] = { pos.x, pos.y, pos.z };
			ImGui::InputFloat3("Position", p, "%.2f", ImGuiInputTextFlags_ReadOnly);

			// サイズ
			Vector3 size = ore->GetSize();
			float s[3] = { size.x, size.y, size.z };
			ImGui::InputFloat3("Size", s, "%.2f", ImGuiInputTextFlags_ReadOnly);

			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	ImGui::Separator();

	bool is = true;
	auto it = oreItems_.find(selectId_);
	if (it == oreItems_.end()) {
		is = false;
	}

	if (is) {
		OreItem* ore = GetOreItemForId();
		// HP
		int hp = ore->GetHp();
		int maxHp = ore->GetMaxHp();
		ImGui::Text("Hp: %d / %d", hp, maxHp);

		// 労働者数
		int currentWorker = ore->GetCurrentWorkerNum();
		int maxWorker = ore->GetMaxWorkerNum();
		ImGui::Text("Workers: %d / %d", currentWorker, maxWorker);

		// 座標
		Vector3 pos = ore->GetPos();
		float p[3] = { pos.x, pos.y, pos.z };
		ImGui::InputFloat3("Position", p, "%.2f", ImGuiInputTextFlags_ReadOnly);

		// サイズ
		Vector3 size = ore->GetSize();
		float s[3] = { size.x, size.y, size.z };
		ImGui::InputFloat3("Size", s, "%.2f", ImGuiInputTextFlags_ReadOnly);
	}
	ImGui::End();
#endif
}

void OreItemManager::AddOreItem(OreType type, const Vector3& pos, const float& rotY) {

	int32_t index = currentId_++;

	switch (type)
	{
	case OreType::Large: {
		// 金鉱石を追加
		std::unique_ptr<GoldOre> ore = std::make_unique<GoldOre>();
		ore->Init(largeDrawData_, largeTexture_, pos, type, rotY);

		oreItems_[index] = std::move(ore);
		break;
	}

	case OreType::Medium: {
		// 金鉱石を追加
		std::unique_ptr<GoldOre> ore = std::make_unique<GoldOre>();
		ore->Init(midleDrawData_, midleTexture_, pos, type, rotY);

		oreItems_[index] = std::move(ore);
		break;
	}

	case OreType::Small: {
		// 金鉱石を追加
		std::unique_ptr<GoldOre> ore = std::make_unique<GoldOre>();
		ore->Init(smallDrawData_, smallTexture_, pos, type, rotY);

		oreItems_[index] = std::move(ore);
		break;
	}
	}

	TextData data;

	// フォント
	data.font = std::make_unique<FontObject>();
	data.font->Initialize(fontName_, L"00/00", fontDrawData_, fontLoader_);
	data.font->transform_.position = pos;
	data.font->transform_.position.x -= 0.8f;
	data.font->transform_.position.y += 3.5f;
	data.font->transform_.position.z += 0.4f;
	data.font->transform_.rotate.x = -2.4f;
	data.font->transform_.scale = Vector3(0.01f, -0.01f,1.0f);
	data.font->fontColor_ = { 0.0f,1.0f,1.0f,1.0f };
	data.font->Update();

	data.fontPos = data.font->transform_.position;

	// 背景
	data.bgSprite = std::make_unique<DefaultObject>();
	data.bgSprite->Initialize(spriteDrawData_, 0);
	data.bgSprite->transform_.position = pos;
	data.bgSprite->transform_.position += Vector3(-0.2f,3.8f,0.4f);
	data.bgSprite->transform_.scale = { 2.0f,1.0f,1.0f };
	data.bgSprite->transform_.rotate.x = -2.4f;
	data.bgSprite->material_.color = { 0.0f,0.0f,0.0f,0.8f };
	data.bgSprite->Update();
	// hp
	data.hpSprite = std::make_unique<DefaultObject>();
	data.hpSprite->Initialize(hpDrawData_, 0);
	data.hpSprite->transform_.position = pos;
	data.hpSprite->transform_.position += Vector3(-1.2f, 3.6f, 0.0f);
	data.hpSprite->transform_.scale = { 2.0f,0.15f,1.0f };
	data.hpSprite->transform_.rotate.x = -2.4f;
	data.hpSprite->material_.color = { 0.0f,1.0f,0.0f,0.8f };
	data.hpSprite->Update();

	// フォントを登録
	fontList_[index] = std::move(data);

	//PointLightの追加
	PointLight light{};
	light.color = ConvertColor(0xcfd942ff);
	light.intensity = 3.0f;
	light.radius = 6.0f;
	light.decay = 6.0f;
	Vector3* position = oreItems_[index]->GetPosPtr();
	int lightHandle = LightManager::GetInstance()->AddPointLight(position, light);
	oreItems_[index]->RegistLightIndex(lightHandle);
}

bool OreItemManager::IsSelectOre(const Vector3 selectpos, Vector3& worldPos) {

	// 選択した位置に鉱石が存在しているかを確認
	for (auto& [id, ore] : oreItems_) {

		Vector3 orePos = ore->GetPos();
		Vector3 oreSize = ore->GetSize();

		if (selectpos.x >= orePos.x - oreSize.x * 0.5f && selectpos.x <= orePos.x + oreSize.x * 0.5f) {
			if (selectpos.z >= orePos.z - oreSize.z * 0.5f && selectpos.z <= orePos.z + oreSize.z * 0.5f) {
				// 移動する位置を設定
				worldPos = orePos;
				selectId_ = id;
				return true;
			}
		}
	}

	// 何もない場合
	selectId_ = -1;
	return false;
}

OreItem* OreItemManager::GetOreItemForId() {
	// 選択した
	return oreItems_[selectId_].get();
}

void OreItemManager::AddTimerUI(const Vector3& pos, OreType oreType) {

	AudioManager::GetInstance().Play(addTimeSH_, 0.5f, false);

	TimerData timerData;

	std::wstring s;

	// 鉱石の大きさによって追加する時間を変更する
	switch (oreType)
	{
	case OreType::Large:
		s = L"+3";
		timeTracker_->AddRemainingTime(3.0f);
		break;

	case OreType::Medium:
		s = L"+2";
		timeTracker_->AddRemainingTime(2.0f);
		break;

	case OreType::Small:
		s = L"+1";
		timeTracker_->AddRemainingTime(1.0f);
		break;
	}

	// 3dUI
	timerData.font3d = std::make_unique<FontObject>();
	timerData.font3d->Initialize(fontName_, s, fontDrawData_, fontLoader_);
	timerData.font3d->transform_.position = pos;
	timerData.font3d->transform_.position.x -= 0.8f;
	timerData.font3d->transform_.position.y += 3.5f;
	timerData.font3d->transform_.position.z += 0.4f;
	timerData.font3d->transform_.rotate.x = -2.4f;
	timerData.font3d->transform_.scale = Vector3(0.02f, -0.02f, 1.0f);
	timerData.font3d->fontColor_ = { 0.0f,1.0f,1.0f,1.0f };
	timerData.font3d->Update();
	// 3dIcon
	timerData.icon3d = std::make_unique<SpriteObject>();
	timerData.icon3d->Initialize(spriteDrawData_, {1.0f,1.0f });
	timerData.icon3d->transform_.position = pos + Vector3(-1.2f,0.0f,0.5f);
	timerData.icon3d->color_ = { 1.0f,1.0f,1.0f,1.0f };
	timerData.icon3d->transform_.rotate.x = 1.4f;
	timerData.icon3d->SetTexture(iconTex_);
	timerData.icon3d->Update();

	timerData.startPosY_ = pos.y;

	// 2dUI
	timerData.font2d = std::make_unique<FontObject>();
	timerData.font2d->Initialize(fontName_, s, fontDrawData_, fontLoader_);
	timerData.font2d->transform_.position = { 1200.0f,128.0f,0.0f };
	timerData.font2d->transform_.scale = Vector3(0.8f, -0.8f, 1.0f);
	timerData.font2d->fontColor_ = { 0.0f,1.0f,1.0f,1.0f };
	timerData.font2d->Update();
	// 2dicon
	timerData.icon2d = std::make_unique<SpriteObject>();
	timerData.icon2d->Initialize(spriteDrawData_, { 64.0f,64.0f });
	timerData.icon2d->transform_.position = { 1200.0f - 48.0f,128.0f,0.0f };
	timerData.icon2d->color_ = { 1.0f,1.0f,1.0f,1.0f };
	timerData.icon2d->SetTexture(iconTex_);
	timerData.icon2d->Update();

	// UIを登録する
	addTimerFontList_.push_back(std::move(timerData));
}

void OreItemManager::TimerUIUpdate() {

	// 終了フラグが立った敵を削除
	addTimerFontList_.remove_if([](const TimerData& data) {
		return data.isFinished_;
	});

	// 更新処理
	for (auto& data : addTimerFontList_) {

		data.timer_ += FpsCount::deltaTime / 1.0f;

		data.font2d->transform_.position.y = lerp(180.0f, 80.0f, data.timer_, EaseType::EaseInCubic);
		data.icon2d->transform_.position.y = lerp(180.0f, 80.0f, data.timer_, EaseType::EaseInCubic);

		data.font3d->transform_.position.y = lerp(data.startPosY_,5.0f,data.timer_,EaseType::EaseInCubic);
		data.icon3d->transform_.position.y = lerp(data.startPosY_,5.0f,data.timer_,EaseType::EaseInCubic);

		// 更新処理
		data.font2d->Update();
		data.font3d->Update();
		data.icon2d->Update();
		data.icon3d->Update();

		if (data.timer_ >= 1.0f) {
			data.isFinished_ = true;
		}
	}
}