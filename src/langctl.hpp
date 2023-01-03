#pragma once

#include "constants.hpp"
#include "natives.hpp"

#include <map>
#include <string>
#include <sstream>
#include <fstream>

typedef struct {
	int f_langCode;
	std::string f_locName;
	std::map<std::string, std::string> f_map;
} LangMap;

typedef std::map<std::string, LangMap> LangsMap;

class LangCtl {
	LangsMap m_lngMap;
	LangMap *m_currLang,
	*m_defaultLang;

public:
	LangCtl() : m_lngMap({}), m_currLang(nullptr), m_defaultLang(nullptr) {};

	template<typename ...Args>
	std::string Get(std::string code, Args... ar) {
		if (auto localLang = m_currLang) {
			lng_tryagain:
			for (auto &it : localLang->f_map) {
				if (it.first == code) {
					auto fmt = it.second.c_str();
					int need = std::snprintf(nullptr, 0, fmt, ar...);
					if (need <= 0) return code;
					auto needsz = static_cast<size_t>(need);
					std::unique_ptr<char[]> b (new char[needsz + 1]);
					std::snprintf(b.get(), needsz + 1, fmt, ar...);
					return std::string{b.get(), needsz};
				}
			}

			if (localLang != m_defaultLang) {
				localLang = m_defaultLang;
				goto lng_tryagain;
			}
		}

		return code;
	}

	std::string Get(std::string code) {
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

	bool Load(std::string lngcode) {
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

	void Change(std::string lng) {
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

	LangsMap &GetMap(void) {
		return m_lngMap;
	}
};

extern LangCtl Lng;
