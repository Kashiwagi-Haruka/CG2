#include "StageNumber.h"

int StageNumber::FromStageName(const std::string& stageName) {
	if (stageName == "MirrorStage") {
		return 1;
	}
	if (stageName == "LightStage") {
		return 2;
	}
	if (stageName == "TutorialStage") {
		return 3;
	}
	if (stageName == "RadiconStage") {
		return 4;
	}

	if (stageName == "RestroomStage") {
		return 6;
	}

	if (stageName == "ElevatorFallStage") {
		return 8;
	}

	if (stageName == "GentleManStage") {
		return 10;
	}
	return 0;
}