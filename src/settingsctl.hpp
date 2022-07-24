#pragma once

#include "thirdparty\json.hpp"

#include <fstream>
#include <string>

class SettingsController {
	std::string m_file;
	nlohmann::json m_data;
	bool m_modified = true;

public:
	SettingsController(std::string file)
		: m_file(file), m_data(R"(
			{
				"menu_hotkey": 118,
				"menu_position": 0,
				"autorun": true
			}
		)"_json) { if(!Load()) Save(); };

	bool Load(void) {
		std::ifstream jfile(m_file);
		if(jfile.is_open()) {
			if(!(m_data = nlohmann::json::parse(jfile, nullptr, false)).is_discarded())
				if(m_modified = !m_data.is_object()) m_data = {}; // Очищаем невалидный конфиг
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
