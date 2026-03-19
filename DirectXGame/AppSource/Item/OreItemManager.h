#pragma once
#include<vector>

#include"Object/OreItem.h"
#include"Object/GoldOre.h"

#include"Object/OreOutLineObject.h"
#include"UI/Object/FontObject.h"
#include"GameCommon/DefaultObject.h"
#include"UI/Object/SpriteObject.h"

#include"Unit/MapChipField.h"
#include"Effect/OreFragmentParticle.h"
#include"Effect/OreBreakParticle.h"
#include"Effect/DirtMoveParticle.h"
#include"TimeTracker.h"

class OreItemManager {
public:

	void Initialize(DrawData spriteDrawData, DrawData hpDrawData,const std::string& fontName, DrawData fontDrawData, FontLoader* fontLoader, DrawData drawData,int stageNum,int icon);

	void Update(bool isOpenMap = false);

	void Draw(Window* window, const Matrix4x4& vpMatrix);

	void DrawUI();

	void DrawTimeUI(Window* window, const Matrix4x4& vpMatrix);

	void DrawEffect(Window* window, const Matrix4x4& vpMatrix);

public:

	void AddOreItem(OreType type, const Vector3& pos, const float& rotY = 0.0f);

	/// <summary>
	/// 選択した位置に鉱石が存在しているかを取得
	/// </summary>
	/// <param name="selectpos">選択する位置</param>
	/// <param name="worldPos">選択した鉱石の位置</param>
	/// <returns></returns>
	bool IsSelectOre(const Vector3 selectpos,Vector3& worldPos);

	// 選択されたIdから鉱石を取得する
	OreItem* GetOreItemForId();

	// 現在の鉱石の数
	int32_t GetCurrentOreItemNum() { return static_cast<int32_t>(oreItems_.size()); }

	// 描画データを設定
	void SetModle(const DrawData& smallDrawData, const DrawData& midleDrawData, const DrawData& largeDrawData, int st,int mt,int lt) {
		smallDrawData_ = smallDrawData;
		midleDrawData_ = midleDrawData;
		largeDrawData_ = largeDrawData;
		smallTexture_ = st;
		midleTexture_ = mt;
		largeTexture_ = lt;
	}

	void SetMapChipField(MapChipField* MapChipField) {
		MapChipField_ = MapChipField;
	}

	// 泥沼のパーティクルを設定
	void SetDirtParticleEmit();

	// 時間管理クラスを取得
	void SetTimeTracker(TimeTracker* timeTracker) {
		timeTracker_ = timeTracker;
	}

private:

	// マップの解釈昨日
	MapChipField* MapChipField_ = nullptr;

	// 時間管理
	TimeTracker* timeTracker_ = nullptr;

	// 鉱石の達
	std::unordered_map<int32_t, std::unique_ptr<OreItem>> oreItems_;
	// 現在のid
	int32_t currentId_ = 0;

	// 鉱石の削除リスト
	std::vector<std::pair<std::unique_ptr<OreItem>, int>> graveyard_;

	// 選択されているidを保持
	int32_t selectId_ = 0;

	// 選択中のオブジェクトにアウトラインを描画する
	std::unique_ptr<OreOutLineObject> oreOutLineObject_;

	// 描画データ
	DrawData smallDrawData_;
	int smallTexture_ = 0;
	DrawData midleDrawData_;
	int midleTexture_ = 0;
	DrawData largeDrawData_;
	int largeTexture_ = 0;

	// 破片パーティクル
	std::unique_ptr<OreFragmentParticle> oreFragmentParticle_;

	// 鉱石の破壊表現
	std::unique_ptr<OreBreakParticle> oreBreakParticle_;

	// 泥のパーティクル
	std::unique_ptr<DirtMoveParticle> dirtMoveParticle_;

	bool isLargeScale_ = false;

	struct DirtData {
		Vector3 pos;
		float timer_ = 0.0f;
		float maxTime_ = 1.0f;
	};

	// 泥の位置リスト
	std::vector<DirtData> dirtDataList_;

	int iconTex_ = 0;

	uint32_t addTimeSH_ = 0;

private: // フォント機能

	struct TextData {
		std::unique_ptr<FontObject> font;
		std::unique_ptr<DefaultObject> hpSprite;
		std::unique_ptr<DefaultObject> bgSprite;
		Vector3 fontPos = { 0,0,0 };
	};

	// 時間の追加UI
	struct TimerData {
		std::unique_ptr<FontObject> font3d;
		std::unique_ptr<SpriteObject> icon3d;
		std::unique_ptr<FontObject> font2d;
		std::unique_ptr<SpriteObject> icon2d;
		float startPosY_ = 0.0f;
		bool isFinished_ = false;
		float timer_ = 0.0f;
	};

	FontLoader* fontLoader_ = nullptr;
	DrawData fontDrawData_;
	DrawData spriteDrawData_;
	DrawData hpDrawData_;

	// フォントデータ
	std::unordered_map<int32_t, TextData> fontList_;

	// 時間追加の描画処理
	std::list<TimerData> addTimerFontList_;

	std::string fontName_;

private:

	// 時間の追加
	void AddTimerUI(const Vector3& pos, OreType oreType);

	// 時間を追加するUIの更新処理
	void TimerUIUpdate();
};