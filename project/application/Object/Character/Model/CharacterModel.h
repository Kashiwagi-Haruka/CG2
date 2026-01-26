#pragma once
#include <string>
#include <memory>
class CharacterModel {

	std::unique_ptr<CharacterModel> instance_;

	public:

	void LoadModel();

};
