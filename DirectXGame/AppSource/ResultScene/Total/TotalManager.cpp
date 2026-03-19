#include "TotalManager.h"

namespace {
	std::string fontName = "YDWbananaslipplus.otf";
}

void TotalManager::Initialize(int maxOre, int getOre, int killOre, int totalGold, int maxGold, const DrawData& drawData, FontLoader* fontLoader) {
	maxOre_ = std::make_unique<FontObject>();
	maxOre_->Initialize(fontName, L"最大Ore数　　　　: " + std::to_wstring(maxOre), drawData, fontLoader);

	getOre_ = std::make_unique<FontObject>();
	getOre_->Initialize(fontName, L"増やしたOreの数　: " + std::to_wstring(getOre), drawData, fontLoader);

	killOre_ = std::make_unique<FontObject>();
	killOre_->Initialize(fontName, L"死んだOreの数　　: " + std::to_wstring(killOre), drawData, fontLoader);

	totalGold_ = std::make_unique<FontObject>();
	totalGold_->Initialize(fontName, L"総獲得Gold: " + std::to_wstring(totalGold), drawData, fontLoader);
	totalGold_->AnchorPoint({ 0.5f, 0.5f });

	maxGold_ = std::make_unique<FontObject>();
	maxGold_->Initialize(fontName, L"フロア最大Gold: " + std::to_wstring(maxGold), drawData, fontLoader);

	uiMessage_ = std::make_unique<FontObject>();
	uiMessage_->Initialize(fontName, L"Press Button to Title", drawData, fontLoader);

	colors_.resize(5);
	Load();
}

void TotalManager::Update(float deltaTime) {
	maxOre_->fontColor_ = colors_[0];
	getOre_->fontColor_ = colors_[1];
	killOre_->fontColor_ = colors_[2];
	totalGold_->fontColor_ = colors_[3];
	maxGold_->fontColor_ = colors_[4];

	t += deltaTime * 1.5f;
	totalGold_->transform_.rotate.z = sinf(t) * rotate_;
}

void TotalManager::Draw(Window* window, const Matrix4x4& vpMatrix) {
	maxOre_->Draw(window, vpMatrix);
	getOre_->Draw(window, vpMatrix);
	killOre_->Draw(window, vpMatrix);
	totalGold_->Draw(window, vpMatrix);
	maxGold_->Draw(window, vpMatrix);
	uiMessage_->Draw(window, vpMatrix);
}

void TotalManager::DrawImGui() {
#ifdef USE_IMGUI

	ImGui::Begin("TotalManager");

	ImGui::DragFloat3("MaxOre Scale", &maxOre_->transform_.scale.x, 0.01f);
	ImGui::DragFloat3("MaxOre Position", &maxOre_->transform_.position.x, 1.0f);
	ImGui::ColorEdit4("MaxOre Color", &colors_[0].x);
	ImGui::Separator();
	ImGui::DragFloat3("GetOre Scale", &getOre_->transform_.scale.x, 0.01f);
	ImGui::DragFloat3("GetOre Position", &getOre_->transform_.position.x, 1.0f);
	ImGui::ColorEdit4("GetOre Color", &colors_[1].x);
	ImGui::Separator();
	ImGui::DragFloat3("KillOre Scale", &killOre_->transform_.scale.x, 0.01f);
	ImGui::DragFloat3("KillOre Position", &killOre_->transform_.position.x, 1.0f);
	ImGui::ColorEdit4("KillOre Color", &colors_[2].x);
	ImGui::Separator();
	ImGui::DragFloat3("TotalGold Scale", &totalGold_->transform_.scale.x, 0.01f);
	ImGui::DragFloat3("TotalGold Position", &totalGold_->transform_.position.x, 1.0f);
	ImGui::ColorEdit4("TotalGold Color", &colors_[3].x);
	ImGui::Separator();
	ImGui::DragFloat3("MaxGold Scale", &maxGold_->transform_.scale.x, 0.01f);
	ImGui::DragFloat3("MaxGold Position", &maxGold_->transform_.position.x, 1.0f);
	ImGui::ColorEdit4("MaxGold Color", &colors_[4].x);
	ImGui::DragFloat3("UIMessage Scale", &uiMessage_->transform_.scale.x, 0.01f);
	ImGui::DragFloat3("UIMessage Position", &uiMessage_->transform_.position.x, 1.0f);

	ImGui::End();

#endif
}

void TotalManager::Save() {
	bManager_.RegisterOutput(maxOre_->transform_.scale);
	bManager_.RegisterOutput(maxOre_->transform_.position);
	bManager_.RegisterOutput(getOre_->transform_.scale);
	bManager_.RegisterOutput(getOre_->transform_.position);
	bManager_.RegisterOutput(killOre_->transform_.scale);
	bManager_.RegisterOutput(killOre_->transform_.position);
	bManager_.RegisterOutput(totalGold_->transform_.scale);
	bManager_.RegisterOutput(totalGold_->transform_.position);
	bManager_.RegisterOutput(maxGold_->transform_.scale);
	bManager_.RegisterOutput(maxGold_->transform_.position);
	for (const auto& color : colors_) {
		bManager_.RegisterOutput(color);
	}
	bManager_.RegisterOutput(uiMessage_->transform_.scale);
	bManager_.RegisterOutput(uiMessage_->transform_.position);
	bManager_.Write(save_);
}

void TotalManager::Load() {
	auto values = bManager_.Read(save_);

	if (values.empty()) {
		return;
	}

	int index = 0;
	maxOre_->transform_.scale = BinaryManager::Reverse<Vector3>(values[index++].get());
	maxOre_->transform_.position = BinaryManager::Reverse<Vector3>(values[index++].get());
	getOre_->transform_.scale = BinaryManager::Reverse<Vector3>(values[index++].get());
	getOre_->transform_.position = BinaryManager::Reverse<Vector3>(values[index++].get());
	killOre_->transform_.scale = BinaryManager::Reverse<Vector3>(values[index++].get());
	killOre_->transform_.position = BinaryManager::Reverse<Vector3>(values[index++].get());
	totalGold_->transform_.scale = BinaryManager::Reverse<Vector3>(values[index++].get());
	totalGold_->transform_.position = BinaryManager::Reverse<Vector3>(values[index++].get());
	maxGold_->transform_.scale = BinaryManager::Reverse<Vector3>(values[index++].get());
	maxGold_->transform_.position = BinaryManager::Reverse<Vector3>(values[index++].get());
	colors_.clear();
	for (int i = 0; i < 5; ++i) {
		colors_.push_back(BinaryManager::Reverse<Vector4>(values[index++].get()));
	}

	if (index < values.size()) {
		uiMessage_->transform_.scale = BinaryManager::Reverse<Vector3>(values[index++].get());
		uiMessage_->transform_.position = BinaryManager::Reverse<Vector3>(values[index++].get());
	}
}
