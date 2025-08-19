#include <map>
#include <fstream>
#include <iostream>
#include <string>
#include <fstream>  
#include <vector>  
#include <iterator>
#include <algorithm>
#include "f4se\GameData.h"
#include "Utils.h"


typedef std::pair<std::string, std::string> entry;


namespace std {
	std::istream& operator >> (std::istream& is, entry& d) {
		std::getline(is, d.first, '=');
		std::getline(is, d.second);
		return is;
	}
}

inline bool exists_file(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

bool bFriendlyNPCMarkers = true;
bool bDistanceBasedMarkers = true;
float fMaxActorCompassDetectionDistance = 5000.00;
float fMinActorCompassDetectionDistance = 1000.00;
float fActorCompassDetectionHostileMult = 2.5;
float fActorCompassDetectionActorValueMult = 0.5;
float fDefaultNPCMarkerScale = 100.00;
float fMinNPCMarkerScale = 60.00;
std::string sAffectingAvifFormId = "Fallout4.esm|2C3";


std::vector<UInt32> ExcludedForms = {};

int formsCounter = 0;

void Getinis(std::vector<WIN32_FIND_DATA>* arr) {
	char* modSettingsDirectory = "Data\\F4SE\\Plugins\\FriendlyNPCsOnCompass.ini";

	HANDLE hFind;
	WIN32_FIND_DATA data;
	hFind = FindFirstFile(modSettingsDirectory, &data);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			arr->push_back(data);
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}
}


void HandleIniFile() {

	if (!exists_file("Data\\F4SE\\Plugins\\FriendlyNPCsOnCompass.ini")) {
		_MESSAGE("ini doesnt exist");
		return;
	}
	std::vector<WIN32_FIND_DATA> modSettingFiles;
	Getinis(&modSettingFiles);
	for (size_t i = 0; i < modSettingFiles.size(); i++)
	{
		std::string fileName = modSettingFiles[i].cFileName;
		std::string fileNameNoSuffix = fileName.substr(0, fileName.size() - 4);

		std::string ff = "Data\\F4SE\\Plugins\\" + fileName;

		std::ifstream in(ff);

		std::map<std::string, std::string> dict((std::istream_iterator<entry>(in)),
			std::istream_iterator<entry>());


		for (entry const& e : dict) {

			if (e.first.find("bDistanceBasedMarkers") != std::string::npos) {

				bDistanceBasedMarkers = (bool)std::stoi(e.second);
				_MESSAGE("bDistanceBasedMarkers %i", std::stoi(e.second));
			}
			else if (e.first.find("bFriendlyNPCMarkers") != std::string::npos) {

				bFriendlyNPCMarkers = (bool)std::stoi(e.second);
				_MESSAGE("bFriendlyNPCMarkers %i", std::stoi(e.second));
			}
			else if (e.first.find("fMaxActorCompassDetectionDistance") != std::string::npos) {

				fMaxActorCompassDetectionDistance = std::stof(e.second);
				_MESSAGE("fMaxActorCompassDetectionDistance %f", fMaxActorCompassDetectionDistance);
			}
			else if (e.first.find("fMinActorCompassDetectionDistance") != std::string::npos) {

				fMinActorCompassDetectionDistance = std::stof(e.second);
				_MESSAGE("fMinActorCompassDetectionDistance %f", fMinActorCompassDetectionDistance);
			}
			else if (e.first.find("fActorCompassDetectionHostileMult") != std::string::npos) {

				fActorCompassDetectionHostileMult = std::stof(e.second);
				_MESSAGE("fActorCompassDetectionHostileMult %f", fActorCompassDetectionHostileMult);
			}
			else if (e.first.find("fActorCompassDetectionActorValueMult") != std::string::npos) {

				fActorCompassDetectionActorValueMult = std::stof(e.second);
				_MESSAGE("fActorCompassDetectionActorValueMult %f", fActorCompassDetectionActorValueMult);
			}
			else if (e.first.find("fDefaultNPCMarkerScale") != std::string::npos) {

				fDefaultNPCMarkerScale = std::stof(e.second);
				_MESSAGE("fDefaultNPCMarkerScale %f", fDefaultNPCMarkerScale);
			}
			else if (e.first.find("fMinNPCMarkerScale") != std::string::npos) {

				fMinNPCMarkerScale = std::stof(e.second);
				_MESSAGE("fMinNPCMarkerScale %f", fMinNPCMarkerScale);
			}

			else if (e.first.find("sAffectingAvifFormId") != std::string::npos) {

				sAffectingAvifFormId = e.second;
				//_MESSAGE("sAffectingAvifFormId %s", sAffectingAvifFormId);
			}


		}

	}
}