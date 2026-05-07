#pragma once

#include <string>

class TxtManager {
public:
	static TxtManager* GetInstance();

	// 指定したtxtファイルを読み込み、内容を1つの文字列として返す
	std::string LoadTxtAsString(const std::string& filePath) const;
	// 指定したtxtファイルを読み込み、UTF-8からu32stringへ変換して返す
	std::u32string LoadTxtAsU32String(const std::string& filePath) const;

private:
	TxtManager() = default;
};