#pragma once

#include "constants.hpp"
#include "thirdparty\json.hpp"

#include <fstream>
#include <string>

class SettingsController {
	std::string m_file;
	nlohmann::json m_data;
	bool m_modified = true;

public:
	SettingsController(std::string file)
		: m_file(file) { if(!Load()) Save(); };

	bool Load(void) {
		std::ifstream jfile(m_file);
		if(jfile.is_open()) {
			if(!(m_data = nlohmann::json::parse(jfile, nullptr, false)).is_discarded())
				if(m_modified = !m_data.is_object()) m_data = {
					{"menu_hotkey", REDLUA_HOTKEY_DEFAULT},
					{"menu_position", 0},
					{"auto_updates", false},
					{"autorun", false},
				}; // Очищаем невалидный конфиг
			jfile.close();

			return true;
		}

		return false;
	}

	std::string &Read(std::string name, std::string &def) {
		if(m_data[name].is_string())
			m_data[name].get_to(def);
		else
			Write(name, def);
		return def;
	}

	int Read(std::string name, int def) {
		if(m_data[name].is_number())
			m_data[name].get_to(def);
		else
			Write(name, def);
		return def;
	}

	bool Read(std::string name, bool def) {
		if(m_data[name].is_boolean())
			m_data[name].get_to(def);
		else
			Write(name, def);
		return def;
	}

	bool Switch(std::string name, bool def) {
		bool state = Read(name, def);
		return Write(name, !state);
	}

	template<typename T>
	T Write(std::string name, T val) {
		m_data[name] = val;
		m_modified = true;
		return val;
	}

	bool Save(void) {
		if(!m_modified) return true;
		std::ofstream jfile(m_file);
		if(jfile.is_open()) {
			jfile << std::setw(4) << m_data << std::endl;
			jfile.close();
			return true;
		}

		return false;
	}
};

extern SettingsController Settings;
