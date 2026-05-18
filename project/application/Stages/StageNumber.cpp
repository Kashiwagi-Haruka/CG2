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
	if (stageName == "GentleManStage") {
		return 5;
	}
	if (stageName == "RestroomStage") {
		return 6;
	}
	return 0;
}