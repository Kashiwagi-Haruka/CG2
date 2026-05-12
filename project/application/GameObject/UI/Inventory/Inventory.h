#pragma once
#include "Engine/Texture/Sprite/Sprite.h"
#include "Item/ItemBase.h"
#include "Text/Text.h"
#include <array>
#include <cstdint>
#include <memory>
#include <string>
class Inventory {

	static const int kMaxItemCount = 8;   // 最大アイテム数
	static const int kItemSlotSize = 100; // アイテムスロットのサイズ

	std::array<std::unique_ptr<Sprite>, kMaxItemCount> itemSlots_; // アイテムスロットのスプライト

	// ▼ 追加：実際に持っているアイテムデータの配列
	std::array<std::shared_ptr<ItemBase>, kMaxItemCount> items_;
	std::unique_ptr<Sprite> selectedSlotSprite_;
	int selectedIndex_ = 0;
	bool isMenuSelecting_ = false;

	// ▼ 追加：各スロットのアイテム所持数を記録する配列
	std::array<int, kMaxItemCount> itemCounts_ = {0};
	int currentItemCount_ = 0; // 現在の所持数
	uint32_t fontHandle_;
	std::array<Text, kMaxItemCount> countTexts_;
	Text useItemText_;
	int useItemTextTimer_ = 0;

public:
	// シングルトン化
	static Inventory* GetInstance() {
		static Inventory instance;
		return &instance;
	}
	Inventory();
	~Inventory();
	void Initialize();
	void Update();
	void Draw();
	// ▼ 追加：アイテムをインベントリに入れる処理
	bool AddItem(std::shared_ptr<ItemBase> item);
	bool ConsumeSelectedItem();
};