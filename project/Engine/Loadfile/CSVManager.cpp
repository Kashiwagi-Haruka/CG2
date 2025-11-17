#include "CSVManager.h"
#include <fstream>
#include <sstream>

CSVManager* CSVManager::GetInstance() {
	static CSVManager instance;
	return &instance;
}

void CSVManager::LoadCSV(const std::string& filename) {

	data_.clear();

	std::ifstream file(filename);
	if (!file.is_open()) {
		printf("CSV ファイルを開けません: %s\n", filename.c_str());
		return;
	}

	std::string line;

	while (std::getline(file, line)) {

		std::stringstream ss(line);
		std::string cell;
		std::vector<int> row;

		while (std::getline(ss, cell, ',')) {
			if (cell.size() > 0) {
				row.push_back(std::stoi(cell));
			}
		}

		if (!row.empty()) {
			data_.push_back(row);
		}
	}

	file.close();
}
