#include "Inventory.h"
#include"TextureManager.h"
#include"SpriteCommon.h"
Inventory::Inventory() {}
Inventory::~Inventory() {}
void Inventory::Initialize() {
	for (int i = 0; i < kMaxItemCount; ++i) {
		itemSlots_[i] = std::make_unique<Sprite>();
        uint32_t handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/Icons/iconFrame.png");
        itemSlots_[i]->Initialize(handle);
		itemSlots_[i]->SetScale({kItemSlotSize, kItemSlotSize});
		itemSlots_[i]->SetPosition({256.0f + i * (kItemSlotSize + 10.0f), 64.0f});
	}
}
void Inventory::Update() {
	for (auto& slot : itemSlots_) {
		slot->Update();
	}
}
void Inventory::Draw() {

    SpriteCommon::GetInstance()->DrawCommon();

	for (auto& slot : itemSlots_) {
		slot->Draw();
	}
}

bool Inventory::AddItem(std::shared_ptr<ItemBase> item)
{
  
    // 1. すでに同じ種類のアイテムを持っているかチェックする
    for (int i = 0; i < currentItemCount_; ++i) {
        if (items_[i]->GetItemID() == item->GetItemID()) {
            // 同じアイテムを見つけた！数を1つ増やして終了
            itemCounts_[i]++;

            // （※後日、画面に「x2」などの数字を描画したい場合は、ここで数字UIを更新します）

            return true; // 追加成功
        }
    }

    // インベントリがいっぱいなら追加できない（falseを返す）
    if (currentItemCount_ >= kMaxItemCount) {
        return false;
    }

    // 1. データを配列に保存する
    items_[currentItemCount_] = item;
    itemCounts_[currentItemCount_] = 1; // 最初の1個目なので「1」にする

    // 2. スプライト（見た目）に、アイテムのテクスチャをセットする
    itemSlots_[currentItemCount_]->SetTextureHandle(item->GetTextureHandle());

    // スプライトの初期化（※必要に応じてInitializeを呼ぶか設定を変更）

    // 
    // 例えば、最初はスプライトを非表示（Colorのアルファ値を0等）にしておいて、
    // ここで1.0fにして表示させる、といった処理を入れます。

    currentItemCount_++; // 所持数を増やす
    return true; // 追加成功！
}
