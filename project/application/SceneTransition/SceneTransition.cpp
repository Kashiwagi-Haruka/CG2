#include "SceneTransition.h"

#include "Color/Color.h"
#include "GameBase.h"
#include "ScreenSize/ScreenSize.h"
#include "Sprite/SpriteCommon.h"
#include "Text/FreetypeManager/FreeTypeManager.h"
#include "TextureManager.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>

SceneTransition::SceneTransition() {
	blockSpriteData_.handle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/white2x2.png");
	blockSpriteData_.sprite = std::make_unique<Sprite>();
	blockSpriteData_.sprite->Initialize(blockSpriteData_.handle);
	blockSpriteData_.sprite->SetColor({0, 0, 0, 1});

	fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
	FreeTypeManager::SetPixelSizes(fontHandle_, 96, 96);

	errorText_.Initialize(fontHandle_);
	errorText_.SetString(U"打刻エラー");
	errorText_.SetPosition({SCREEN_SIZE::HALF_WIDTH, SCREEN_SIZE::HALF_HEIGHT});
	errorText_.SetAlign(TextAlign::Center);
	errorText_.SetColor(COLOR::WHITE);
	errorText_.SetBlendMode(BlendMode::kBlendModeAlpha);

	blocks_.clear();
	blockOrder_.clear();
}

SceneTransition::~SceneTransition() {}

void SceneTransition::Initialize(bool isIn) {
	isIn_ = isIn;
	isEnd = false;
	holdTimer_ = 0.0f;
	typeStarted_ = false;
	errorText_.UpdateLayout(true);

	if (isIn_) {
		StartSpread(false);
	} else {
		StartSpread(true);
	}
}

void SceneTransition::StartSpread(bool reverseStart) {
	phaseTimer_ = 0.0f;
	phase_ = reverseStart ? Phase::Reverse : Phase::Spread;
	SetupBlocks();
}

void SceneTransition::SetupBlocks() {
	std::mt19937 mt(std::random_device{}());

	const int columns = static_cast<int>(std::ceil(SCREEN_SIZE::WIDTH / blockSize_)) + 2;
	const int rows = static_cast<int>(std::ceil(SCREEN_SIZE::HEIGHT / blockSize_)) + 2;
	const size_t blockCount = static_cast<size_t>(columns * rows);

	blocks_.resize(blockCount);
	blockOrder_.resize(blockCount);
	std::iota(blockOrder_.begin(), blockOrder_.end(), 0);
	std::shuffle(blockOrder_.begin(), blockOrder_.end(), mt);

	const float halfBlock = blockSize_ * 0.5f;
	for (size_t i = 0; i < blockCount; ++i) {
		const int x = static_cast<int>(i % columns);
		const int y = static_cast<int>(i / columns);
		const float positionX = x * blockSize_ - halfBlock;
		const float positionY = y * blockSize_ - halfBlock;

		Block& block = blocks_[i];
		block.position = {positionX, positionY};
		block.size = blockSize_ * 1.2f;

		const float progress = static_cast<float>(blockOrder_[i]) / static_cast<float>(std::max<size_t>(blockCount - 1, 1));
		block.delay = progress * blockDelayMax_;
	}
}

float SceneTransition::GetPhaseRatio(float duration) const {
	if (duration <= 0.0f) {
		return 1.0f;
	}
	return std::clamp(phaseTimer_ / duration, 0.0f, 1.0f);
}

void SceneTransition::Update() {
	if (isEnd) {
		return;
	}

	switch (phase_) {
	case Phase::Spread:
		UpdateSpread();
		break;
	case Phase::HoldAndType:
		UpdateHoldAndType();
		break;
	case Phase::Reverse:
		UpdateReverse();
		break;
	}
}

void SceneTransition::UpdateSpread() {
	phaseTimer_ += GameBase::GetInstance()->GetDeltaTime();
	if (GetPhaseRatio(spreadDuration_) >= 1.0f) {
		phase_ = Phase::HoldAndType;
		holdTimer_ = 0.0f;
		phaseTimer_ = 0.0f;
	}
}

void SceneTransition::UpdateHoldAndType() {
	holdTimer_ += GameBase::GetInstance()->GetDeltaTime();
	if (!typeStarted_ && holdTimer_ >= textStartDelay_) {
		errorText_.StartTyping(0.08f);
		typeStarted_ = true;
	}
	if (typeStarted_) {
		errorText_.Update(true);
	}

	constexpr float kTypingDuration = 1.2f;
	if (holdTimer_ >= textStartDelay_ + kTypingDuration + afterTextWait_) {
		phase_ = Phase::Reverse;
		phaseTimer_ = 0.0f;
	}
}

void SceneTransition::UpdateReverse() {
	phaseTimer_ += GameBase::GetInstance()->GetDeltaTime();
	if (GetPhaseRatio(reverseDuration_) >= 1.0f) {
		isEnd = true;
	}
}

void SceneTransition::Draw() {
	SpriteCommon::GetInstance()->DrawCommon();
	SpriteCommon::GetInstance()->SetBlendMode(BlendMode::kBlendModeAlpha);

	float t = 0.0f;
	if (phase_ == Phase::Spread) {
		t = GetPhaseRatio(spreadDuration_);
	} else if (phase_ == Phase::HoldAndType) {
		t = 1.0f;
	} else {
		t = 1.0f - GetPhaseRatio(reverseDuration_);
	}

	t = std::clamp(t, 0.0f, 1.0f);

	if (t > 0.0f) {
		blockSpriteData_.sprite->SetColor({0.0f, 0.0f, 0.0f, t});
		blockSpriteData_.sprite->SetScale({SCREEN_SIZE::WIDTH, SCREEN_SIZE::HEIGHT});
		blockSpriteData_.sprite->SetPosition({0.0f, 0.0f});
		blockSpriteData_.sprite->Update();
		blockSpriteData_.sprite->Draw();
	}

		blockSpriteData_.sprite->SetColor({0.0f, 0.0f, 0.0f, 1.0f});

	for (const auto& block : blocks_) {
		const float appear = std::clamp((t - block.delay) / (1.0f - block.delay + 0.0001f), 0.0f, 1.0f);
		if (appear <= 0.0f) {
			continue;
		}
		const float drawSize = block.size * appear;
		blockSpriteData_.sprite->SetScale({drawSize, drawSize});
		blockSpriteData_.sprite->SetPosition({block.position.x - drawSize * 0.5f, block.position.y - drawSize * 0.5f});
		blockSpriteData_.sprite->Update();
		blockSpriteData_.sprite->Draw();
	}

	if (phase_ == Phase::HoldAndType || (phase_ == Phase::Reverse && t > 0.8f)) {
		errorText_.Draw();
	}
}