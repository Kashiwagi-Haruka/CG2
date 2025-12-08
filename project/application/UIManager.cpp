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

	HowtoOperateSPData[kAttuckButton].handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/uvChecker.png");

	// Level
	LevelSPData.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/Lv.png");

	// 数字テクスチ
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

	// ===========================
	//      Sprite の生成
	// ===========================

	playerHpSPData.sprite = new Sprite();
	playerHPFlameSPData.sprite = new Sprite();

	for (int i = 0; i < OperateCountMAX; i++)
		HowtoOperateSPData[i].sprite = new Sprite();

	LevelSPData.sprite = new Sprite();

	for (int i = 0; i < NumbersCountMax; i++)
		NumberSPData[i].sprite = new Sprite();
	for (int i = 0; i < 5; i++) {
	MaxSPData[i].sprite = new Sprite();
	}

	AttuckUpSPData.sprite = new Sprite();
	HealthUpSPData.sprite = new Sprite();
	SpeedUpSPData.sprite = new Sprite();
	AllowUpSPData.sprite = new Sprite();
}

UIManager::~UIManager() {

	delete playerHpSPData.sprite;
	delete playerHPFlameSPData.sprite;

	for (int i = 0; i < OperateCountMAX; i++)
		delete HowtoOperateSPData[i].sprite;

	delete LevelSPData.sprite;

	for (int i = 0; i < NumbersCountMax; i++)
		delete NumberSPData[i].sprite;
	for (int i = 0; i < 5; i++) {
		delete MaxSPData[i].sprite;
	}
	delete AttuckUpSPData.sprite;
	delete HealthUpSPData.sprite;
	delete SpeedUpSPData.sprite;
	delete AllowUpSPData.sprite;
}

void UIManager::Initialize() {

	auto spriteCommon = GameBase::GetInstance()->GetSpriteCommon();

	// ------------------ HP Bar ------------------
	playerHpSPData.sprite->Initialize(spriteCommon, playerHpSPData.handle);
	playerHpSPData.size = playerHPMaxSize;
	playerHpSPData.sprite->SetScale(playerHpSPData.size);
	playerHpSPData.sprite->SetPosition(playerHpSPData.translate);

	// ------------------ HP Flame ------------------
	playerHPFlameSPData.sprite->Initialize(spriteCommon, playerHPFlameSPData.handle);
	playerHPFlameSPData.size = {40, 70};
	playerHPFlameSPData.sprite->SetScale(playerHPFlameSPData.size);

	// ------------------ WASD / SPACE / ATTACK ------------------
	for (int i = 0; i < OperateCountMAX; i++) {
		HowtoOperateSPData[i].sprite->Initialize(spriteCommon, HowtoOperateSPData[i].handle);
		HowtoOperateSPData[i].sprite->SetScale({40, 40});
	}

	// ------------------ Level ------------------
	LevelSPData.sprite->Initialize(spriteCommon, LevelSPData.handle);
	LevelSPData.sprite->SetScale({50, 50});
	LevelSPData.translate = {10, 10};
	LevelSPData.sprite->SetPosition(LevelSPData.translate);

	// ------------------ Number 0〜9 ------------------
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
	
}

void UIManager::Update() {

	// ---- HP ----
	playerHpSPData.size.y = (playerHPMaxSize.y / playerHPMax) * playerHP;
	playerHpSPData.sprite->SetScale(playerHpSPData.size);

	// Flame 位置調整
	playerHPFlameSPData.sprite->SetPosition({playerHpSPData.translate.x - 10, playerHpSPData.translate.y - 10});

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
	//     EXP（3桁表示）
	// ===========================

	int exp = parameters_.EXP;

	int exp100 = exp / 100;      // 百の位
	int exp10 = (exp / 10) % 10; // 十の位
	int exp1 = exp % 10;         // 一の位

	NumberSPData[kExp100].sprite->SetTextureRange({300.0f * exp100, 0}, numbersTextureSize);
	NumberSPData[kExp10].sprite->SetTextureRange({300.0f * exp10, 0}, numbersTextureSize);
	NumberSPData[kEexp1].sprite->SetTextureRange({300.0f * exp1, 0}, numbersTextureSize);

	// ===========================
	//     MaxEXP（3桁表示）
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

	NumberSPData[kLv].translate = {LevelSPData.translate.x + 50, LevelSPData.translate.y +12};
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

		NumberSPData[i].sprite->SetPosition({i*50.0f+100,20});
	}

	for (int i = 0; i < NumbersCountMax; i++){
		NumberSPData[i].sprite->Update();
	}
	MaxSPData[0].translate = {NumberSPData[kLv].translate.x, NumberSPData[kLv].translate.y-10};
	MaxSPData[1].translate = {NumberSPData[kAttuck].translate.x,NumberSPData[kAttuck].translate.y-10};
	MaxSPData[2].translate = {NumberSPData[kHealth].translate.x, NumberSPData[kHealth].translate.y-10};
	MaxSPData[3].translate = {NumberSPData[kSpeed].translate.x, NumberSPData[kSpeed].translate.y-10};
	MaxSPData[4].translate = {NumberSPData[kArrow].translate.x,NumberSPData[kArrow].translate.y-10};

	for (int i = 0; i < 5; i++) {

	MaxSPData[i].sprite->SetPosition(MaxSPData[i].translate);
	MaxSPData[i].sprite->Update();
	}
}

void UIManager::Draw() {

	GameBase::GetInstance()->SpriteCommonSet();

	playerHpSPData.sprite->Draw();
	playerHPFlameSPData.sprite->Draw();

	for (int i = 0; i < kAttuckButton; i++)
		HowtoOperateSPData[i].sprite->Draw();

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
	
	
	
}

void UIManager::SetPlayerPosition(Vector2 playerPosition){ 
	playerHpSPData.translate.x = playerPosition.x+600;
	playerHpSPData.translate.y = playerPosition.y + 500;
}
void UIManager::SetPlayerHP(int HP) { playerHP = HP; }
void UIManager::SetPlayerHPMax(int HPMax) { playerHPMax = HPMax; }
void UIManager::SetPlayerParameters(Parameters parameters) { parameters_ = parameters; }
