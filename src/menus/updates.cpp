#include "menus\updates.hpp"
#include "menus\link.hpp"
#include "settingsctl.hpp"
#include "constants.hpp"

#include "thirdparty\json.hpp"
#include <Windows.h>

using json = nlohmann::json;

static const char *symlist[] = {
	"curl_easy_init", "curl_easy_setopt",
	"curl_easy_perform", "curl_easy_strerror",
	"curl_easy_getinfo", "curl_easy_cleanup",
	"curl_slist_append", "curl_slist_free_all",
	NULL
};

static const char *liblist[] = {
	"libcurl-x64.dll",
	"libcurl.dll",
	"curl.dll",
	NULL
};

static struct {
	HMODULE lib;

	void *(*easy_init)(void);
	int (*easy_setopt)(void *, int, ...);
	int (*easy_perform)(void *);
	const char *(*easy_strerror)(int);
	void (*easy_getinfo)(void *, int, void *);
	void (*easy_cleanup)(void *);
	void *(*slist_append)(void *, const char *);
	void (*slist_free_all)(void *);
} curl;

static bool load_curl(void) {
	for(int i = 0; !curl.lib && liblist[i]; i++) {
		curl.lib = LoadLibraryA(liblist[i]);
		for(int j = 0; curl.lib && symlist[j]; j++) {
			if((((void**)&curl)[j + 1] = GetProcAddress(curl.lib, symlist[j])) == NULL) {
				FreeLibrary(curl.lib);
				curl.lib = NULL;
				break;
			}
		}
	}

	return curl.lib != NULL;
}

class MenuUpdatePrompt : public MenuBase {
	virtual void OnPop(void) {
		delete this;
	}

public:
	MenuUpdatePrompt(MenuItemTitle *title)
		: MenuBase(title) {}
};

static const std::string errors[] = {
	"Error: ~COLOR_RED~libcurl.dll not found!",
	"Server responded with error: ~COLOR_RED~",
	"HTTP request failed: ~COLOR_RED~",
	"Malformed server response"
};

class MenuItemUpdateRL : public MenuItemDefault {
	static size_t __stdcall write_string(char *ptr, size_t sz, size_t nmemb, void *ud) {
		*(static_cast<std::string*>(ud)) += std::string{ptr, sz * nmemb};
		return sz * nmemb;
	}

	virtual void OnSelect(void) {
		if(!load_curl()) {
			SetStatusText(errors[0]);
			return;
		}

		std::string data;
		void *slist = NULL;
		slist = curl.slist_append(slist, "User-Agent: RL/1.0");
		slist = curl.slist_append(slist, "Accept: application/json");

		void *handle = curl.easy_init();
		curl.easy_setopt(handle, 42/*CURLOPT_HEADER*/, 0);
		curl.easy_setopt(handle, 52/*CURLOPT_FOLLOWLOCATION*/, 1);
		curl.easy_setopt(handle, 53/*CURLOPT_TRANSFERTEXT*/, 1);
		curl.easy_setopt(handle, 64/*CURLOPT_SSL_VERIFYPEER*/, 0);
		curl.easy_setopt(handle, 10002/*CURLOPT_URL*/, REDLUA_TAGS_URL);
		curl.easy_setopt(handle, 10023/*CURLOPT_HTTPHEADER*/, slist);
		curl.easy_setopt(handle, 20011/*CURLOPT_WRITEFUNCTION*/, write_string);
		curl.easy_setopt(handle, 10001/*CURLOPT_WRITEDATA*/, &data);

		int res;
		if((res = curl.easy_perform(handle)) == 0) {
			json jdata = json::parse(data, nullptr, false);
			if(!jdata.is_discarded()) {
				std::string temp;
				std::string newest;
				int curr_rel = -1;

				if(jdata.is_array()) {
					for(auto &x : jdata.items()) {
						json &jname = x.value()["name"];
						if(jname.is_string()) {
							int vidx = std::strtoul(x.key().c_str(), nullptr, 10);
							if(jname.get_to(temp) == REDLUA_VERSION)
								curr_rel = vidx;
							if(vidx == 0)
								newest = temp;
						}
					}
				} else if(jdata.is_object() && jdata["message"].is_string())
					SetStatusText(errors[1] + jdata["message"].get_to(temp));

				if(newest != "" && curr_rel != 0) {
					auto menu = new MenuUpdatePrompt(new MenuItemTitle("New version " + newest + " found!"));
					auto controller = GetMenu()->GetController();
					controller->RegisterMenu(menu);

					menu->AddItem(new MenuItemLink("Open download page", (std::string)REDLUA_RELS_URL + newest));
					controller->PushMenu(menu);
				}
			} else
				SetStatusText(errors[2] + errors[3]);
		} else
			SetStatusText(errors[2] + (std::string)curl.easy_strerror(res));
		
		curl.slist_free_all(slist);
		curl.easy_cleanup(handle);
	}

public:
	MenuItemUpdateRL(std::string title)
		: MenuItemDefault(title) {}
};

class MenuItemUpdateDB : public MenuItemDefault {
	static size_t etag_extract(char *ptr, size_t sz, size_t nmemb, void *ud) {
		size_t out = sz * nmemb;
		if(out > 10 && _strnicmp(ptr, "etag: ", 6) == 0) {
			std::string *str = (std::string *)ud;
			// Вот такой вот странный, но действенный способ обрезать символы \r\n
			*str = std::string{ptr + 6, out - (ptr[out - 2] == '\r' ? 8 : (ptr[out - 1] == '\n' ? 7 : 6))};
		}

		return out;
	}

	virtual void OnSelect(void) {
		if(!load_curl()) {
			SetStatusText(errors[0]);
			return;
		}

		FILE *file = fopen(REDLUA_NATIVES_FILE, "r+");
		if(!file && (file = fopen(REDLUA_NATIVES_FILE, "w")) == NULL) {
			SetStatusText("Failed to open " REDLUA_NATIVES_FILE " file!");
			return;
		}

		std::string etag = "W/\"\"";
		void *slist = NULL;
		slist = curl.slist_append(slist, "User-Agent: RL/1.0");
		slist = curl.slist_append(slist, "Accept: application/json");
		slist = curl.slist_append(slist, ("If-None-Match: " + Settings.Read("nativedb_etag", etag)).c_str());

		void *handle = curl.easy_init();
		curl.easy_setopt(handle, 42/*CURLOPT_HEADER*/, 0);
		curl.easy_setopt(handle, 52/*CURLOPT_FOLLOWLOCATION*/, 1);
		curl.easy_setopt(handle, 53/*CURLOPT_TRANSFERTEXT*/, 1);
		curl.easy_setopt(handle, 64/*CURLOPT_SSL_VERIFYPEER*/, 0);
		curl.easy_setopt(handle, 10002/*CURLOPT_URL*/, REDLUA_NATIVEDB_URL);
		curl.easy_setopt(handle, 10023/*CURLOPT_HTTPHEADER*/, slist);
		curl.easy_setopt(handle, 20011/*CURLOPT_WRITEFUNCTION*/, fwrite);
		curl.easy_setopt(handle, 10001/*CURLOPT_WRITEDATA*/, file);
		curl.easy_setopt(handle, 20079/*CURLOPT_HEADERFUNCTION*/, etag_extract);
		curl.easy_setopt(handle, 10029/*CURLOPT_WRITEDATA*/, &etag);

		int res;
		if((res = curl.easy_perform(handle)) == 0) {
			long resp = 0;
			curl.easy_getinfo(handle, 0x200002/*CURLINFO_RESPONSE_CODE*/, &resp);
			if(resp >= 200 && resp < 300) {
				Settings.Write("nativedb_etag", "W/" + etag);
				SetStatusText("NativeDB updated, please hit \"Reload NativeDB\" button to apply changes!");
			} else if(resp == 304)
				SetStatusText("You already have the latest version of NativeDB");
			else
				SetStatusText(errors[2] + std::to_string(resp));
		} else
			SetStatusText(errors[2] + (std::string)curl.easy_strerror(res));

		curl.slist_free_all(slist);
		curl.easy_cleanup(handle);
		fclose(file);
	}

public:
	MenuItemUpdateDB(std::string title)
		: MenuItemDefault(title) {}
};

MenuBase *CreateUpdatesMenu(MenuController *controller) {
	auto menu = new MenuBase(new MenuItemTitle("RedLua Settings"));
	controller->RegisterMenu(menu);
	
	menu->AddItem(new MenuItemUpdateRL("Check for RedLua updates"));
	menu->AddItem(new MenuItemUpdateDB("Check for NativeDB updates"));

	return menu;
}
