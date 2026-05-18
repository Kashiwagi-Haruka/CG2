#pragma once
#include <string>

class StageNumber {
public:
	static int FromStageName(const std::string& stageName);
};