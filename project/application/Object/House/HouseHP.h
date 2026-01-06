#pragma once
#include <memory>
class HouseHP {

	static std::unique_ptr<HouseHP> instance;



	public:

	static HouseHP* GetInstance();

};
