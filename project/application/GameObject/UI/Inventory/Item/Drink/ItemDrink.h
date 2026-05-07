#pragma once
#include "GameObject/UI/Inventory/Item/ItemBase.h"
#include "GameObject/VendingMac/Drink/Drink.h" // DrinkNameを使うため

class ItemDrink : public ItemBase {
public:
    // コンストラクタでドリンクの種類を受け取る
    ItemDrink(Drink::DrinkName type);
    // オーバーライドしてテクスチャハンドルを返す
    uint32_t GetTextureHandle() const override;
    int GetItemID() const override;
private:
    Drink::DrinkName type_;
    uint32_t textureHandle_; // 2Dアイコン用のテクスチャ
};