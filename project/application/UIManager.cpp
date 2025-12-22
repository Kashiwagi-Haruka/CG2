#define NOMINMAX
#include "UIManager.h"
#include "GameBase.h"
#include "Sprite.h"
#include "TextureManager.h"

UIManager::UIManager() {

	// ===========================
	//      テクスチャ読み込み
	// ===========================

	playerHpSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/playerHP.png");

	playerHPFlameSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/playerHPFlame.png");

	// WASD / SPACE / ATTACK
	HowtoOperateSPData[kW].handle = HowtoOperateSPData[kA].handle = HowtoOperateSPData[kS].handle = HowtoOperateSPData[kD].handle =
	    TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/WASD.png");

	HowtoOperateSPData[kSpace].handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/SPACE.png");

	HowtoOperateSPData[kAttuckButton].handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/J.png");

	// Level
	LevelSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/Lv.png");

	// 数字テクスチャ
	for (int i = 0; i < NumbersCountMax; i++) {
		NumberSPData[i].handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/No.png");
	}
	for (int i = 0; i < 5; i++) {
		MaxSPData[i].handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/Max.png");
	}

	// ステータスUPアイコン
	AttuckUpSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/AttuckUp.png");

	HealthUpSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/HealthUp.png");

	SpeedUpSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/SpeedUp.png");

	AllowUpSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/ArrowUp.png");

	SlashSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/Slash.png");

	EXPSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/Exp.png");

	// ===========================
	//      Sprite の生成
	// ===========================

	playerHpSPData.sprite = std::make_unique<Sprite>();
	playerHPFlameSPData.sprite = std::make_unique<Sprite>();

	for (int i = 0; i < OperateCountMAX; i++)
		HowtoOperateSPData[i].sprite = std::make_unique<Sprite>();

	LevelSPData.sprite = std::make_unique<Sprite>();

	for (int i = 0; i < NumbersCountMax; i++)
		NumberSPData[i].sprite = std::make_unique<Sprite>();
	for (int i = 0; i < 5; i++) {
		MaxSPData[i].sprite = std::make_unique<Sprite>();
	}

	AttuckUpSPData.sprite = std::make_unique<Sprite>();
	HealthUpSPData.sprite = std::make_unique<Sprite>();
	SpeedUpSPData.sprite = std::make_unique<Sprite>();
	AllowUpSPData.sprite = std::make_unique<Sprite>();
	houseHpSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/playerHP.png");

	houseHpFlameSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/playerHPFlame.png");

	houseHpSPData.sprite = std::make_unique<Sprite>();
	houseHpFlameSPData.sprite = std::make_unique<Sprite>();
}

UIManager::~UIManager() {}

void UIManager::Initialize() {

	auto spriteCommon = GameBase::GetInstance()->GetSpriteCommon();

	// ------------------ HP Bar ------------------
	playerHpSPData.sprite->Initialize(spriteCommon, playerHpSPData.handle);
	playerHpSPData.size = playerHPMaxSize;
	// ★ アンカーポイントを左端に設定（左端固定）
	playerHpSPData.sprite->SetAnchorPoint({0.0f, 0.0f});
	playerHpSPData.sprite->SetScale(playerHpSPData.size);
	// ★ 左端の位置を固定
	playerHpSPData.translate = {640 - playerHPMaxSize.x / 2, 600};
	playerHpSPData.sprite->SetPosition(playerHpSPData.translate);

	// ------------------ HP Flame ------------------
	playerHPFlameSPData.sprite->Initialize(spriteCommon, playerHPFlameSPData.handle);
	playerHPFlameSPData.size = playerHPMaxSize;
	playerHPFlameSPData.translate = {640, 600};
	playerHPFlameSPData.sprite->SetAnchorPoint({0.5f, 0.0f});
	playerHPFlameSPData.sprite->SetPosition(playerHPFlameSPData.translate);
	playerHPFlameSPData.sprite->SetScale(playerHPFlameSPData.size);

	// ------------------ WASD / SPACE / ATTACK ------------------
	for (int i = 0; i < OperateCountMAX; i++) {
		HowtoOperateSPData[i].sprite->Initialize(spriteCommon, HowtoOperateSPData[i].handle);
		HowtoOperateSPData[i].sprite->SetScale({100, 100});
		HowtoOperateSPData[i].sprite->SetTextureRange({0, 0}, {768, 768});
	}
	HowtoOperateSPData[kW].sprite->SetPosition({10, 600});
	HowtoOperateSPData[kSpace].sprite->SetPosition({130, 600});
	HowtoOperateSPData[kAttuckButton].sprite->SetPosition({250, 600});

	// ------------------ Level ------------------
	LevelSPData.sprite->Initialize(spriteCommon, LevelSPData.handle);
	LevelSPData.sprite->SetScale({50, 50});
	LevelSPData.translate = {10, 10};
	LevelSPData.sprite->SetPosition(LevelSPData.translate);

	// ------------------ Number 0～9 ------------------
	for (int i = 0; i < NumbersCountMax; i++) {
		NumberSPData[i].sprite->Initialize(spriteCommon, NumberSPData[i].handle);
		NumberSPData[i].sprite->SetTextureRange({0, 0}, numbersTextureSize);
		NumberSPData[i].sprite->SetScale({32, 32});
	}
	for (int i = 0; i < 5; i++) {
		MaxSPData[i].sprite->Initialize(spriteCommon, MaxSPData[i].handle);

		MaxSPData[i].sprite->SetScale({48, 48});
	}
	// Up アイコン
	AttuckUpSPData.sprite->Initialize(spriteCommon, AttuckUpSPData.handle);
	AttuckUpSPData.sprite->SetScale({48, 48});

	HealthUpSPData.sprite->Initialize(spriteCommon, HealthUpSPData.handle);
	HealthUpSPData.sprite->SetScale({48, 48});

	SpeedUpSPData.sprite->Initialize(spriteCommon, SpeedUpSPData.handle);
	SpeedUpSPData.sprite->SetScale({48, 48});

	AllowUpSPData.sprite->Initialize(spriteCommon, AllowUpSPData.handle);
	AllowUpSPData.sprite->SetScale({48, 48});

	// --- House HP Bar ---
	houseHpSPData.sprite->Initialize(spriteCommon, houseHpSPData.handle);
	houseHpSPData.size = {120, 20}; // ★ 横長に
	houseHpSPData.sprite->SetScale(houseHpSPData.size);

	// 位置(自由に調整)：画面左上あたり
	houseHpSPData.translate = {50, 550};
	houseHpSPData.sprite->SetPosition(houseHpSPData.translate);

	// --- House HP Flame ---
	houseHpFlameSPData.sprite->Initialize(spriteCommon, houseHpFlameSPData.handle);
	houseHpFlameSPData.size = {130, 30};
	houseHpFlameSPData.sprite->SetScale(houseHpFlameSPData.size);
	houseHpFlameSPData.translate = {houseHpSPData.translate.x - 5, houseHpSPData.translate.y};
	houseHpFlameSPData.sprite->SetPosition(houseHpFlameSPData.translate);

	// ★ 90度回転(横ゲージ化)
	houseHpSPData.sprite->SetRotation({90, 0});
	houseHpFlameSPData.sprite->SetRotation({90, 0});
}

void UIManager::Update() {

	// ---- HP (左端固定、右端から減る) ----
	if (playerHPMax > 0) {
		// HP比率を計算
		float hpRatio = (float)playerHP / (float)playerHPMax;

		// スケールを更新（HPに応じて幅を変更）
		playerHpSPData.size.x = playerHPMaxSize.x * hpRatio;
		playerHpSPData.sprite->SetScale(playerHpSPData.size);

		// テクスチャ範囲を更新（左端0から、HP比率分だけ表示）
		playerHPWidth = playerHPWidthMax * hpRatio;
		playerHpSPData.sprite->SetTextureRange({0, 0}, {playerHPWidth, 300});

		// ★ 左端の位置は常に固定（ずれない）
		playerHpSPData.translate = {640 - playerHPMaxSize.x / 2, 600};
		playerHpSPData.sprite->SetPosition(playerHpSPData.translate);
	}

	// 各スプライト Update
	playerHpSPData.sprite->Update();
	playerHPFlameSPData.sprite->Update();

	for (int i = 0; i < OperateCountMAX; i++)
		HowtoOperateSPData[i].sprite->Update();

	LevelSPData.sprite->Update();

	AttuckUpSPData.translate = {10, 70};
	AttuckUpSPData.sprite->SetPosition(AttuckUpSPData.translate);
	AttuckUpSPData.sprite->Update();
	HealthUpSPData.translate = {10, AttuckUpSPData.translate.y + 70};
	HealthUpSPData.sprite->SetPosition(HealthUpSPData.translate);
	HealthUpSPData.sprite->Update();
	SpeedUpSPData.translate = {10, HealthUpSPData.translate.y + 70};
	SpeedUpSPData.sprite->SetPosition(SpeedUpSPData.translate);
	SpeedUpSPData.sprite->Update();
	AllowUpSPData.translate = {10, SpeedUpSPData.translate.y + 70};
	AllowUpSPData.sprite->SetPosition(AllowUpSPData.translate);
	AllowUpSPData.sprite->Update();

	// ===========================
	//     EXP(3桁表示)
	// ===========================

	int exp = parameters_.EXP;

	int exp100 = exp / 100;      // 百の位
	int exp10 = (exp / 10) % 10; // 十の位
	int exp1 = exp % 10;         // 一の位

	NumberSPData[kExp100].sprite->SetTextureRange({300.0f * exp100, 0}, numbersTextureSize);
	NumberSPData[kExp10].sprite->SetTextureRange({300.0f * exp10, 0}, numbersTextureSize);
	NumberSPData[kEexp1].sprite->SetTextureRange({300.0f * exp1, 0}, numbersTextureSize);

	// ===========================
	//     MaxEXP(3桁表示)
	// ===========================

	int expMax = parameters_.MaxEXP;

	int expMax100 = expMax / 100;
	int expMax10 = (expMax / 10) % 10;
	int expMax1 = expMax % 10;

	NumberSPData[kExpMax100].sprite->SetTextureRange({300.0f * expMax100, 0}, numbersTextureSize);
	NumberSPData[kExpMax10].sprite->SetTextureRange({300.0f * expMax10, 0}, numbersTextureSize);
	NumberSPData[kExpMax1].sprite->SetTextureRange({300.0f * expMax1, 0}, numbersTextureSize);

	NumberSPData[kLv].sprite->SetTextureRange({300.0f * parameters_.Level, 0}, numbersTextureSize);
	NumberSPData[kAttuck].sprite->SetTextureRange({300.0f * parameters_.AttuckUp, 0}, numbersTextureSize);
	NumberSPData[kHealth].sprite->SetTextureRange({300.0f * parameters_.HPUp, 0}, numbersTextureSize);
	NumberSPData[kSpeed].sprite->SetTextureRange({300.0f * parameters_.SpeedUp, 0}, numbersTextureSize);
	NumberSPData[kArrow].sprite->SetTextureRange({300.0f * parameters_.AllowUp, 0}, numbersTextureSize);

	NumberSPData[kLv].translate = {LevelSPData.translate.x + 50, LevelSPData.translate.y + 12};
	NumberSPData[kLv].sprite->SetPosition(NumberSPData[kLv].translate);

	NumberSPData[kAttuck].translate = {AttuckUpSPData.translate.x + 40, AttuckUpSPData.translate.y + 20};
	NumberSPData[kAttuck].sprite->SetPosition(NumberSPData[kAttuck].translate);

	NumberSPData[kHealth].translate = {HealthUpSPData.translate.x + 40, HealthUpSPData.translate.y + 20};
	NumberSPData[kHealth].sprite->SetPosition(NumberSPData[kHealth].translate);

	NumberSPData[kSpeed].translate = {SpeedUpSPData.translate.x + 40, SpeedUpSPData.translate.y + 20};
	NumberSPData[kSpeed].sprite->SetPosition(NumberSPData[kSpeed].translate);

	NumberSPData[kArrow].translate = {AllowUpSPData.translate.x + 40, AllowUpSPData.translate.y + 20};
	NumberSPData[kArrow].sprite->SetPosition(NumberSPData[kArrow].translate);

	for (int i = kExp100; i <= kExpMax1; i++) {

		NumberSPData[i].sprite->SetPosition({i * 50.0f + 100, 20});
	}

	for (int i = 0; i < NumbersCountMax; i++) {
		NumberSPData[i].sprite->Update();
	}
	MaxSPData[0].translate = {NumberSPData[kLv].translate.x, NumberSPData[kLv].translate.y - 10};
	MaxSPData[1].translate = {NumberSPData[kAttuck].translate.x, NumberSPData[kAttuck].translate.y - 10};
	MaxSPData[2].translate = {NumberSPData[kHealth].translate.x, NumberSPData[kHealth].translate.y - 10};
	MaxSPData[3].translate = {NumberSPData[kSpeed].translate.x, NumberSPData[kSpeed].translate.y - 10};
	MaxSPData[4].translate = {NumberSPData[kArrow].translate.x, NumberSPData[kArrow].translate.y - 10};

	for (int i = 0; i < 5; i++) {

		MaxSPData[i].sprite->SetPosition(MaxSPData[i].translate);
		MaxSPData[i].sprite->Update();
	}

	// ---- House HP ----
	if (houseHPMax > 0) {
		float ratio = (float)houseHP / (float)houseHPMax;
		houseHpSPData.size.x = 120 * ratio;
		houseHpSPData.sprite->SetScale(houseHpSPData.size);
	}

	houseHpSPData.sprite->Update();
	houseHpFlameSPData.sprite->Update();
}

void UIManager::Draw() {

	GameBase::GetInstance()->SpriteCommonSet();

	playerHpSPData.sprite->Draw();
	playerHPFlameSPData.sprite->Draw();

	HowtoOperateSPData[kW].sprite->Draw();
	HowtoOperateSPData[kSpace].sprite->Draw();
	HowtoOperateSPData[kAttuckButton].sprite->Draw();

	LevelSPData.sprite->Draw();

	AttuckUpSPData.sprite->Draw();
	HealthUpSPData.sprite->Draw();
	SpeedUpSPData.sprite->Draw();
	AllowUpSPData.sprite->Draw();
	for (int i = 0; i <= kExpMax1; i++) {
		NumberSPData[i].sprite->Draw();
	}
	if (parameters_.Level < parameters_.MaxLevel) {
		NumberSPData[kLv].sprite->Draw();
	} else {
		MaxSPData[0].sprite->Draw();
	}
	if (parameters_.AttuckUp < 10) {
		NumberSPData[kAttuck].sprite->Draw();
	} else {
		MaxSPData[1].sprite->Draw();
	}
	if (parameters_.HPUp < 10) {
		NumberSPData[kHealth].sprite->Draw();
	} else {
		MaxSPData[2].sprite->Draw();
	}
	if (parameters_.SpeedUp < 10) {
		NumberSPData[kSpeed].sprite->Draw();
	} else {
		MaxSPData[3].sprite->Draw();
	}
	if (parameters_.AllowUp < 10) {
		NumberSPData[kArrow].sprite->Draw();
	} else {
		MaxSPData[4].sprite->Draw();
	}

	houseHpFlameSPData.sprite->Draw();
	houseHpSPData.sprite->Draw();
}

void UIManager::SetPlayerHP(int HP) { playerHP = HP; }
void UIManager::SetPlayerHPMax(int HPMax) { playerHPMax = HPMax; }
void UIManager::SetPlayerParameters(Parameters parameters) { parameters_ = parameters; }