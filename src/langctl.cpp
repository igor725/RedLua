#include "langctl.hpp"
#include "constants.hpp"
#include "natives.hpp"

LangCtl Lng;

std::string LangCtl::Get(std::string code) {
	if (auto localLang = m_currLang) {
		lngr_tryagain:
		for (auto &it : localLang->f_map)
			if (it.first == code) return it.second;

		if (localLang != m_defaultLang) {
			localLang = m_defaultLang;
			goto lngr_tryagain;
		}
	}

	return code;
}

bool LangCtl::Load(std::string lngcode) {
	std::ifstream lfile (REDLUA_LANGS_DIR + lngcode + ".lng");
	if (!lfile.is_open()) return false;

	LangMap &lm = m_lngMap[lngcode];
	if (!std::getline(lfile, lm.f_locName) || !lm.f_locName.length()) {
		m_lngMap.erase(lngcode);
		lfile.close();
		return false;
	}
	lfile >> lm.f_langCode;

	for (std::string line; std::getline(lfile, line);) {
		if (line.length() > 0 && line.at(0) != '#') {
			std::stringstream ls(line);
			std::string lncode;
			if (std::getline(ls, lncode, '=')) {
				std::getline(ls, lm.f_map[lncode]);
				continue;
			}

			return false;
		}
	}

	if (lngcode == "en" || !m_defaultLang)
		m_defaultLang = &lm;

	return true;
}

void LangCtl::Change(std::string lng) {
	m_currLang = nullptr;
	if (lng == "ingame") {
		for (auto &it : m_lngMap) {
			int code = NATIVES::GET_LOCALE();
			if (it.second.f_langCode == code) {
				m_currLang = &it.second;
				break;
			}
		}
	} else {
		for (auto &it : m_lngMap) {
			if (it.first == lng) {
				m_currLang = &it.second;
				break;
			}
		}
	}
}

LangsMap &LangCtl::GetMap(void) {
	return m_lngMap;
}
