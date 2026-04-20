#pragma once
#include <array>
#include <memory>
#include "Engine/Texture/Sprite/Sprite.h"
class Inventory {

	static const int kMaxItemCount = 10; // 最大アイテム数
	static const int kItemSlotSize = 100; // アイテムスロットのサイズ

	std::array<std::unique_ptr<Sprite>, kMaxItemCount> itemSlots_; // アイテムスロットのスプライト

public:
	Inventory();
	~Inventory();
	void Initialize();
	void Update();
	void Draw();
};
