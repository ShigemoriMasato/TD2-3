#pragma once
#include <chrono>
#include"TimeLimit.h"

class TimeTracker {
public:

	TimeTracker();

	/// <summary>
	/// 計測する時間を設定
	/// </summary>
	/// <param name="m">分</param>
	/// <param name="s">秒</param>
	void SetCountTime(const float& m, const float& s);

	// 時間の計測を開始
	void StartMeasureTimes();

	// 時間の計測を終了
	void EndMeasureTimes();

	// 時間を0にリセット
	void Reset();

	// 更新処理
	void Update();

	// 終了フラグを取得
	bool isFinishd() const { return isFinished_;}

	// 時間を加算する
	void AddRemainingTime(const float& seconds);

private:

	// 最初の時間
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime_;

	// 最後の時間
	std::chrono::time_point<std::chrono::high_resolution_clock> endTime_;

	// 経過時間を保存
	std::chrono::duration<float> duration_;

	// これまでの蓄積時間を保存
	std::chrono::duration<float> accumulatedTime_;

	// カウントダウンの初期設定時間
	std::chrono::duration<float> initialDuration_;

	bool isFinished_ = false;

	// 計測時間の取得
	bool isRunning_ = false;
};