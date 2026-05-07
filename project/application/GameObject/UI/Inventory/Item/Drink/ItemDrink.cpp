#include "ItemDrink.h"
#include "TextureManager.h"

ItemDrink::ItemDrink(Drink::DrinkName type) : type_(type) {
    // ドリンクの種類に応じて、2D用のアイコン画像を読み込む
    // ※パスはご自身の環境に合わせて変更してください

 

    switch (type_)
    {
    case Drink::WATER:
        textureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/Icons/waterIcon.png");
        break;
    case Drink::TEA:
        textureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/Icons/teaIcon.png");
        break;
    case Drink::COFFEE:
        textureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/Icons/coffeeIcon.png");
        break;
    case Drink::ENERGY:
        textureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/Icons/energyDrinkIcon.png");
        break;
    case Drink::SeveredHead:
        textureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/Icons/severedHeadIcon.png");
        break;
    default:
        //何もタイプが分からない場合水にしておく
        textureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/Icons/waterIcon.png");
        break;
    }

}

uint32_t ItemDrink::GetTextureHandle() const {
    return textureHandle_;
}

int ItemDrink::GetItemID() const
{
    // ドリンクの種類(0, 1, 2...)をそのまま返す。
     // ※今後「鍵(Key)」などが追加された時にIDが被らないよう、
     // ドリンク系は「100番台」にするなどルールを決めると安全です。
    return 100 + type_;
}
