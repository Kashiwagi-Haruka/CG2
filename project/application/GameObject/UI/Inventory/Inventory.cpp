#include "Inventory.h"

Inventory::Inventory() {}
Inventory::~Inventory() {}
void Inventory::Initialize() {
	for (int i = 0; i < kMaxItemCount; ++i) {
		itemSlots_[i] = std::make_unique<Sprite>();
		itemSlots_[i]->SetScale({kItemSlotSize, kItemSlotSize});
		itemSlots_[i]->SetPosition({10.0f + i * (kItemSlotSize + 10.0f), 10.0f});
	}
}
void Inventory::Update() {
	for (auto& slot : itemSlots_) {
		slot->Update();
	}
}
void Inventory::Draw() {
	for (auto& slot : itemSlots_) {
		slot->Draw();
	}
}