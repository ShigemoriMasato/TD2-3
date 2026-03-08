#include"TimeTracker.h"

TimeTracker::TimeTracker() {
	isRunning_ = false;
	accumulatedTime_ = std::chrono::duration<float>::zero();
	duration_ = std::chrono::duration<float>::zero();
	initialDuration_ = std::chrono::duration<float>::zero();
}

void TimeTracker::SetCountTime(const float& m, const float& s) {
	float totalSec = (m * 60.0f) + s;
	initialDuration_ = std::chrono::duration<float>(totalSec);

	// 現在の残り時間を初期時間にする
	duration_ = initialDuration_;

	// 画面表示を更新
	TimeLimit::totalSeconds = static_cast<int>(duration_.count());
	TimeLimit::CreateNumbers();
}

void TimeTracker::StartMeasureTimes() {
	if (!isRunning_) {
		startTime_ = std::chrono::high_resolution_clock::now();
		isRunning_ = true;
	}
}

void TimeTracker::EndMeasureTimes() {
	if (isRunning_) {
		// 最後の時間
		endTime_ = std::chrono::high_resolution_clock::now();

		// 今回の計測区間（End - Start）を蓄積時間に加算する
		accumulatedTime_ += (endTime_ - startTime_);

		isRunning_ = false;

		// 最終的な合計時間を更新
		duration_ = initialDuration_ - accumulatedTime_;

		if (duration_.count() < 0.0f) {
			duration_ = std::chrono::duration<float>::zero();
		}

		TimeLimit::totalSeconds = static_cast<int>(duration_.count());
		// 時間から各桁の番号を取得
		TimeLimit::CreateNumbers();
	}
}

void TimeTracker::Reset() {
	// 計測を停止し、時間を0に戻す
	isRunning_ = false;
	accumulatedTime_ = std::chrono::duration<float>::zero();
	duration_ = initialDuration_;
	// 表示も0に更新
	TimeLimit::totalSeconds = 0;
	TimeLimit::CreateNumbers();
}

void TimeTracker::Update() {
	if (isRunning_) {
		// 現在の時間を取得
		auto currentTime = std::chrono::high_resolution_clock::now();

		// 現在の総経過時間 = 過去の蓄積 + 今回の経過
		auto currentElapsed = accumulatedTime_ + (currentTime - startTime_);

		// 残り時間を計算： 初期時間 - 総経過時間
		duration_ = initialDuration_ - currentElapsed;

		if (duration_.count() <= 0.0f) {
			duration_ = std::chrono::duration<float>::zero();
			isFinished_ = true;
		}

		// TimeLimitのstatic変数を更新
		TimeLimit::totalSeconds = static_cast<int>(duration_.count());
		TimeLimit::CreateNumbers();
	}
}

void TimeTracker::AddRemainingTime(const float& seconds) {
	initialDuration_ += std::chrono::duration<float>(seconds);

	// 既に時間が0になって終了フラグが立っていた場合、時間がプラスに戻るならフラグを解除する
	if (isFinished_) {
		// 現在の総経過時間を計算
		auto currentElapsed = accumulatedTime_;
		if (isRunning_) {
			currentElapsed += (std::chrono::high_resolution_clock::now() - startTime_);
		}

		// 新しい残り時間が0より大きければ終了フラグを解除
		if ((initialDuration_ - currentElapsed).count() > 0.0f) {
			isFinished_ = false;
		}
	}

	// タイマーが停止中の場合は、即座に duration_ と画面表示を更新しておく
	if (!isRunning_) {
		duration_ = initialDuration_ - accumulatedTime_;

		if (duration_.count() < 0.0f) {
			duration_ = std::chrono::duration<float>::zero();
		}

		TimeLimit::totalSeconds = static_cast<int>(duration_.count());
		TimeLimit::CreateNumbers();
	}
}