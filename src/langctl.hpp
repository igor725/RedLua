#pragma once

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

	std::string Get(std::string code);

	bool Load(std::string lngcode);

	void Change(std::string lng);

	LangsMap &GetMap(void);
};

extern LangCtl Lng;
