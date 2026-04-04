#pragma once

#include "Stages/BaseStage.h"

class MirrorStage : public BaseStage {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
};