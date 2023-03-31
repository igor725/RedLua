#pragma once

#include "constants.hpp"
#include "json.hpp"

#include <string>

class SettingsController {
	std::string m_file;
	json m_data;
	bool m_modified = true;

public:
	SettingsController(std::string file)
		: m_file(file) { if (!Load()) Save(); };

	bool Load(void);

	std::string &Read(std::string name, std::string &def);
	int Read(std::string name, int def);
	bool Read(std::string name, bool def);

	bool Switch(std::string name, bool def);

	template<typename T>
	T Write(std::string name, T val) {
		m_data[name] = val;
		m_modified = true;
		return val;
	}

	bool Save(void);
};

extern SettingsController Settings;
