#include "Inventory.h"
#include"TextureManager.h"
#include"SpriteCommon.h"
Inventory::Inventory() {

    fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
    FreeTypeManager::SetPixelSizes(fontHandle_, 32, 32);

}
Inventory::~Inventory() {}
void Inventory::Initialize() {

    currentItemCount_ = 0;

    for (int i = 0; i < kMaxItemCount; ++i) {
        itemSlots_[i] = std::make_unique<Sprite>();
        uint32_t handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/Icons/iconFrame.png");
        itemSlots_[i]->Initialize(handle);
        itemSlots_[i]->SetScale({ kItemSlotSize, kItemSlotSize });
        itemSlots_[i]->SetPosition({ 256.0f+ i * (kItemSlotSize + 10.0f), 64.0f});
        countTexts_[i].Initialize(fontHandle_);
        countTexts_[i].SetString(U"x1");
        Vector2 textPos = itemSlots_[i]->GetTranslate();
        textPos.x += 64.0f;
        textPos.y += 128.0f-32.0f;
        countTexts_[i].SetPosition(textPos);
        countTexts_[i].SetColor({ 1, 1, 1, 1 });
        countTexts_[i].SetAlign(TextAlign::Left);
        countTexts_[i].SetBlendMode(BlendMode::kBlendModeAlpha);
        countTexts_[i].UpdateLayout(false);
        itemCounts_[i] = 0;
        items_[i].reset();
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

    // ▼ 変更：インデックスを使って回すように変更（座標計算などでiが必要になるため）
    for (int i = 0; i < kMaxItemCount; ++i) {
        // ▼ 追加：アイテムを持っている（個数が1以上）場合のみテキストを描画する
        if (itemCounts_[i] > 0) {
            countTexts_[i].Draw();
        }
    }


    SpriteCommon::GetInstance()->DrawCommon();
}

bool Inventory::AddItem(std::shared_ptr<ItemBase> item)
{

    // 1. すでに同じ種類のアイテムを持っているかチェックする
    for (int i = 0; i < currentItemCount_; ++i) {
        if (items_[i] != nullptr && items_[i]->GetItemID() == item->GetItemID()) {
            // 同じアイテムを見つけた！数を1つ増やして終了
            itemCounts_[i]++;
            std::string string = "x" + std::to_string(itemCounts_[i]);
            // （※後日、画面に「x2」などの数字を描画したい場合は、ここで数字UIを更新します）
            countTexts_[i].SetString(std::u32string(string.begin(), string.end()));
            countTexts_[i].UpdateLayout(false);
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

    currentItemCount_++; // 所持数を増やす
    return true; // 追加成功！
}
