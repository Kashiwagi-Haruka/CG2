#include "Inventory.h"
#include "GameObject/KeyBindConfig.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
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
		itemSlots_[i]->SetScale({kItemSlotSize, kItemSlotSize});
		itemSlots_[i]->SetPosition({256.0f + i * (kItemSlotSize + 10.0f), 64.0f});
		countTexts_[i].Initialize(fontHandle_);
		countTexts_[i].SetString(U"x1");
		Vector2 textPos = itemSlots_[i]->GetTranslate();
		textPos.x += 64.0f;
		textPos.y += 128.0f - 32.0f;
		countTexts_[i].SetPosition(textPos);
		countTexts_[i].SetColor({1, 1, 1, 1});
		countTexts_[i].SetAlign(TextAlign::Left);
		countTexts_[i].SetBlendMode(BlendMode::kBlendModeAlpha);
		countTexts_[i].UpdateLayout(false);
		itemCounts_[i] = 0;
		items_[i].reset();
	}

	selectedSlotSprite_ = std::make_unique<Sprite>();
	uint32_t selectedHandle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/white2x2.png");
	selectedSlotSprite_->Initialize(selectedHandle);
	selectedSlotSprite_->SetScale({kItemSlotSize + 20.0f, kItemSlotSize + 20.0f});
	selectedSlotSprite_->SetColor({1.0f, 1.0f, 0.0f, 0.65f});
	selectedSlotSprite_->SetPosition(itemSlots_[0]->GetTranslate());

	useItemText_.Initialize(fontHandle_);
	useItemText_.SetString(U"アイテムを使った");
	useItemText_.SetPosition({256.0f, 180.0f});
	useItemText_.SetColor({1, 1, 1, 1});
	useItemText_.SetAlign(TextAlign::Left);
	useItemText_.SetBlendMode(BlendMode::kBlendModeAlpha);
	useItemText_.UpdateLayout(false);
}
void Inventory::Update() {
	auto* playerCommand = PlayerCommand::GetInstance();
	if (playerCommand->MenuCommandTrigger() && currentItemCount_ > 0) {
		isMenuSelecting_ = !isMenuSelecting_;
	}

	if (isMenuSelecting_) {
		if (playerCommand->UiMoveLeftTrigger()) {
			selectedIndex_--;
			if (selectedIndex_ < 0) {
				selectedIndex_ = currentItemCount_ - 1;
			}
		}
		if (playerCommand->UiMoveRightTrigger()) {
			selectedIndex_++;
			if (selectedIndex_ >= currentItemCount_) {
				selectedIndex_ = 0;
			}
		}
		if (playerCommand->ItemUseTrigger()) {
			if (ConsumeSelectedItem()) {
				useItemTextTimer_ = 120;
			}
		}
	}

	if (useItemTextTimer_ > 0) {
		useItemTextTimer_--;
	}

	if (selectedSlotSprite_ && currentItemCount_ > 0) {
		selectedSlotSprite_->SetPosition(itemSlots_[selectedIndex_]->GetTranslate());
		selectedSlotSprite_->Update();
	}

	for (auto& slot : itemSlots_) {
		slot->Update();
	}
}
void Inventory::Draw() {

	SpriteCommon::GetInstance()->DrawCommon();

	for (auto& slot : itemSlots_) {
		slot->Draw();
	}

	if (isMenuSelecting_ && currentItemCount_ > 0 && selectedSlotSprite_) {
		selectedSlotSprite_->Draw();
	}

	// ▼ 変更：インデックスを使って回すように変更（座標計算などでiが必要になるため）
	for (int i = 0; i < kMaxItemCount; ++i) {
		// ▼ 追加：アイテムを持っている（個数が1以上）場合のみテキストを描画する
		if (itemCounts_[i] > 0) {
			countTexts_[i].Draw();
		}
	}

	if (useItemTextTimer_ > 0) {
		useItemText_.Draw();
	}

	SpriteCommon::GetInstance()->DrawCommon();
}

bool Inventory::AddItem(std::shared_ptr<ItemBase> item) {

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
	return true;         // 追加成功！
}

bool Inventory::ConsumeSelectedItem() {
	if (currentItemCount_ <= 0) {
		return false;
	}

	if (itemCounts_[selectedIndex_] <= 0) {
		return false;
	}

	itemCounts_[selectedIndex_]--;
	if (itemCounts_[selectedIndex_] > 0) {
		std::string string = "x" + std::to_string(itemCounts_[selectedIndex_]);
		countTexts_[selectedIndex_].SetString(std::u32string(string.begin(), string.end()));
		countTexts_[selectedIndex_].UpdateLayout(false);
		return true;
	}

	for (int i = selectedIndex_; i < currentItemCount_ - 1; ++i) {
		items_[i] = items_[i + 1];
		itemCounts_[i] = itemCounts_[i + 1];
		if (items_[i] != nullptr) {
			itemSlots_[i]->SetTextureHandle(items_[i]->GetTextureHandle());
		}
		std::string string = "x" + std::to_string(itemCounts_[i]);
		countTexts_[i].SetString(std::u32string(string.begin(), string.end()));
		countTexts_[i].UpdateLayout(false);
	}

	items_[currentItemCount_ - 1].reset();
	itemCounts_[currentItemCount_ - 1] = 0;
	uint32_t frameHandle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/Icons/iconFrame.png");
	itemSlots_[currentItemCount_ - 1]->SetTextureHandle(frameHandle);
	currentItemCount_--;

	if (currentItemCount_ <= 0) {
		isMenuSelecting_ = false;
		selectedIndex_ = 0;
	} else if (selectedIndex_ >= currentItemCount_) {
		selectedIndex_ = currentItemCount_ - 1;
	}

	return true;
}