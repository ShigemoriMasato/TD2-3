#include "KeyManager.h"

void KeyManager::Initialize(Input* input) {
	input_ = input;
	keyHistory_.resize(kMaxHistory_);
	buttonHistory_.resize(kMaxHistory_);
	stickHistory_.resize(kMaxHistory_);
	mouseHistory_.resize(kMaxHistory_);
}

void KeyManager::Update() {
	// ================- keyの初期化 -================
	for (int i = 0; i < int(Key::Count); i++) {
		//最初に全部falseにしておく
		resultKeyFlugs_[Key(i)] = false;
	}

	// ==========================- キーの状態の更新 -==========================	
	//一番古いものを削除して、新しいものを追加する
	keyHistory_.erase(keyHistory_.begin());
	keyHistory_.emplace_back();

	buttonHistory_.erase(buttonHistory_.begin());
	buttonHistory_.emplace_back();

	stickHistory_.erase(stickHistory_.begin());
	stickHistory_.emplace_back();

	mouseHistory_.erase(mouseHistory_.begin());
	mouseHistory_.emplace_back();

	//Keyの数だけループ
	for (const auto& [action, keys] : keyMap_) {
		//Keyに登録されているDIKの数だけループ
		for (const auto& [key, keyState] : keys) {
			//履歴を作成する
			keyHistory_.back()[key] = input_->GetKeyState(key);
		}
	}

	for (const auto& [action, buttons] : buttonMap_) {
		//Keyに登録されているDIKの数だけループ
		for (const auto& [button, buttonState] : buttons) {
			//履歴を作成する
			buttonHistory_.back()[button] = input_->GetXBoxButtonState(button);
		}
	}

	for (const auto& [action, stick] : stickMap_) {
		//履歴を作成する
		stickHistory_.back()[stick.first].x = input_->GetXBoxStickState(int(stick.first)).x;
		stickHistory_.back()[stick.first].y = input_->GetXBoxStickState(int(stick.first)).y;
	}

	for (const auto& [action, buttons] : mouseMap_) {
		//Keyに登録されているDIKの数だけループ
		for (const auto& [button, buttonState] : buttons) {
			//履歴を作成する
			mouseHistory_.back()[button] = input_->GetMouseButtonState()[button];
		}
	}

	// ================- keyの最終的な状態の更新 -================
	for (const auto& [action, key] : keyMap_) {

		//すでにtrueになっていたらスキップ
		if (resultKeyFlugs_[action]) {
			continue;
		}

		//Keyに登録されているDIKの数だけループ
		for (const auto& [dik, targetState] : key) {

			//最終的な状態
			bool state = false;

			switch (targetState) {
			case KeyState::None:
				state = !keyHistory_.back()[dik];
				break;
			case KeyState::Trigger:
				state = keyHistory_.back()[dik] && !keyHistory_[keyHistory_.size() - 2][dik];
				break;
			case KeyState::Hold:
				state = keyHistory_.back()[dik];
				break;
			case KeyState::Release:
				state = !keyHistory_.back()[dik] && keyHistory_[keyHistory_.size() - 2][dik];
				break;
			}

			//trueになったら登録してループを抜ける
			if (state) {
				resultKeyFlugs_[action] = state;
				break;
			}

		}
	}

	for (const auto& [action, buttons] : buttonMap_) {
		//すでにtrueになっていたらスキップ
		if (resultKeyFlugs_[action]) {
			continue;
		}
		//Keyに登録されているDIKの数だけループ
		for (const auto& [button, targetState] : buttons) {
			//最終的な状態
			bool state = false;
			switch (targetState) {
			case KeyState::None:
				state = !buttonHistory_.back()[button];
				break;
			case KeyState::Trigger:
				state = buttonHistory_.back()[button] && !buttonHistory_[buttonHistory_.size() - 2][button];
				break;
			case KeyState::Hold:
				state = buttonHistory_.back()[button];
				break;
			case KeyState::Release:
				state = !buttonHistory_.back()[button] && buttonHistory_[buttonHistory_.size() - 2][button];
				break;
			}
			//trueになったら登録してループを抜ける
			if (state) {
				resultKeyFlugs_[action] = state;
				break;
			}
		}
	}

	for (const auto& [action, stick] : stickMap_) {
		//すでにtrueになっていたらスキップ
		if (resultKeyFlugs_[action]) {
			continue;
		}

		//最終的な状態
		bool state = false;

		///xかyか
		int isX = static_cast<int>(stick.second.first);
		//目標のスティックの倒れ具合
		float toggleValue = stick.second.second;
		//現在のスティックの倒れ具合
		float currentValue = (&stickHistory_.back()[stick.first].x)[isX];

		//スティックの倒れ具合がtoggleValueを超えたらtrueにする
		if (toggleValue > 0) {
			//右スティック
			state = (currentValue > toggleValue);
		} else {
			//左スティック
			state = (currentValue < toggleValue);
		}

		if (state) {
			resultKeyFlugs_[action] = state;
		}
	}

	for(const auto& [action, buttons] : mouseMap_) {
		
		//すでにtrueになっていたらスキップ
		if (resultKeyFlugs_[action]) {
			continue;
		}

		//Keyに登録されているDIKの数だけループ
		for (const auto& [button, targetState] : buttons) {
			//最終的な状態
			bool state = false;
			switch (targetState) {
			case KeyState::None:
				state = !mouseHistory_.back()[button];
				break;
			case KeyState::Trigger:
				state = mouseHistory_.back()[button] && !mouseHistory_[mouseHistory_.size() - 2][button];
				break;
			case KeyState::Hold:
				state = mouseHistory_.back()[button];
				break;
			case KeyState::Release:
				state = !mouseHistory_.back()[button] && mouseHistory_[mouseHistory_.size() - 2][button];
				break;
			}
			//trueになったら登録してループを抜ける
			if (state) {
				resultKeyFlugs_[action] = state;
				break;
			}
		}
	}
}

void KeyManager::SetKey(Key action, uint8_t DIK, KeyState state) {
	//すでに登録されていたら何もしない
	for (const auto& [key, keyState] : keyMap_[action]) {
		if (key == DIK && keyState == state) {
			return;
		}
	}

	//登録されていなかったら登録する
	keyMap_[action].emplace_back(DIK, state);
}

void KeyManager::SetButton(Key action, XBoxController button, KeyState state) {
	buttonMap_[action].emplace_back(button, state);
}

void KeyManager::SetStick(Key action, bool isLeft, bool isY, float toggleValue) {
	stickMap_[action] = { isLeft ? Direction::Left : Direction::Right, {isY, toggleValue} };
}

void KeyManager::SetMouse(Key action, int mouseButton, KeyState state) {
	mouseMap_[action].emplace_back(mouseButton, state);
}
