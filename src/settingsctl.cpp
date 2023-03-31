#include "settingsctl.hpp"
#include "constants.hpp"
#include <fstream>

SettingsController Settings (REDLUA_SETTINGS_FILE);

bool SettingsController::Load(void) {
	std::ifstream jfile(m_file);
	if (jfile.is_open()) {
		if (!(m_data = json::parse(jfile, nullptr, false)).is_discarded())
			if (m_modified = !m_data.is_object()) goto resetcfg;
		jfile.close();

		return true;
	}

	resetcfg:
	m_data = {
		{"menu_hotkey", REDLUA_HOTKEY_DEFAULT},
		{"menu_language", "ingame"},
		{"menu_position", 0},
		{"auto_updates", false},
		{"autorun", false},
	}; // Очищаем невалидный конфиг
	return false;
}

std::string &SettingsController::Read(std::string name, std::string &def) {
	if (m_data[name].is_string())
		m_data[name].get_to(def);
	else
		Write(name, def);
	return def;
}

int SettingsController::Read(std::string name, int def) {
	if (m_data[name].is_number())
		m_data[name].get_to(def);
	else
		Write(name, def);
	return def;
}

bool SettingsController::Read(std::string name, bool def) {
	if (m_data[name].is_boolean())
		m_data[name].get_to(def);
	else
		Write(name, def);
	return def;
}

bool SettingsController::Switch(std::string name, bool def) {
	bool state = Read(name, def);
	return Write(name, !state);
}

bool SettingsController::Save(void) {
	if (!m_modified) return true;
	std::ofstream jfile(m_file);
	if (jfile.is_open()) {
		jfile << std::setw(4) << m_data << std::endl;
		jfile.close();
		return true;
	}

	return false;
}
