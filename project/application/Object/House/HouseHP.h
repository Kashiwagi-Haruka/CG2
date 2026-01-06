#pragma once
#include <memory>
class HouseHP {

	static std::unique_ptr<HouseHP> instance;

	int HP_;

	public:

	static HouseHP* GetInstance();
	void SetHP(int HP) { HP_ = HP; };
	int GetHP() { return HP_; }

};
