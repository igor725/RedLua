#include "updatesctl.hpp"
#include "constants.hpp"
#include "settingsctl.hpp"

#include "thirdparty\json.hpp"
#include <Windows.h>
#include <string>

UpdatesController UpdatesCtl;
using json = nlohmann::json;

static const char *symlist[] = {
	"curl_easy_init", "curl_easy_setopt",
	"curl_easy_perform", "curl_easy_strerror",
	"curl_easy_getinfo", "curl_easy_cleanup",
	"curl_slist_append", "curl_slist_free_all",
	nullptr
};

static const char *liblist[] = {
	"libcurl-x64.dll",
	"libcurl.dll",
	"curl.dll",
	nullptr
};

static const std::string errors[] = {
	"Error: libcurl.dll not found!",
	"Server responded with error: ",
	"HTTP request failed: ",
	"Malformed server response"
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

bool UpdatesController::Prepare(void) {
	for(int i = 0; !curl.lib && liblist[i]; i++) {
		curl.lib = LoadLibraryA(liblist[i]);
		for(int j = 0; curl.lib && symlist[j]; j++) {
			if((((void**)&curl)[j + 1] = GetProcAddress(curl.lib, symlist[j])) == nullptr) {
				FreeLibrary(curl.lib);
				curl.lib = nullptr;
				break;
			}
		}
	}

	return curl.lib != nullptr;
}

static void *default_headers(void) {
	void *list = curl.slist_append(nullptr, "User-Agent: RL/1.0");
	list = curl.slist_append(list, "Accept: application/json");
	return list;
}

static size_t __stdcall write_string(char *ptr, size_t sz, size_t nmemb, void *ud) {
	*(static_cast<std::string*>(ud)) += std::string{ptr, sz * nmemb};
	return sz * nmemb;
}

bool UpdatesController::CheckRedLua(std::string &ret) {
	if(curl.lib == nullptr && !Prepare()) {
		ret = errors[0];
		return false;
	}

	bool success;
	std::string temp;
	void *slist = default_headers();
	void *handle = curl.easy_init();
	curl.easy_setopt(handle, 42/*CURLOPT_HEADER*/, 0);
	curl.easy_setopt(handle, 52/*CURLOPT_FOLLOWLOCATION*/, 1);
	curl.easy_setopt(handle, 53/*CURLOPT_TRANSFERTEXT*/, 1);
	curl.easy_setopt(handle, 64/*CURLOPT_SSL_VERIFYPEER*/, 0);
	curl.easy_setopt(handle, 10002/*CURLOPT_URL*/, REDLUA_TAGS_URL);
	curl.easy_setopt(handle, 10023/*CURLOPT_HTTPHEADER*/, slist);
	curl.easy_setopt(handle, 20011/*CURLOPT_WRITEFUNCTION*/, write_string);
	curl.easy_setopt(handle, 10001/*CURLOPT_WRITEDATA*/, &temp);

	do {
		int res;
		if((res = curl.easy_perform(handle)) != 0/*CURLE_OK*/) {
			ret = errors[2] + (std::string)curl.easy_strerror(res);
			success = false;
			break;
		}
		json jdata = json::parse(temp, nullptr, false);
		if(jdata.is_discarded()) {
			ret = errors[2] + errors[3];
			success = false;
			break;
		}
		int curr_rel = -1;

		if(jdata.is_array()) {
			for(auto &x : jdata.items()) {
				json &jname = x.value()["name"];
				if(jname.is_string()) {
					int vidx = std::strtoul(x.key().c_str(), nullptr, 10);
					if(jname.get_to(temp) == REDLUA_VERSION)
						curr_rel = vidx;
					if(vidx == 0)
						ret = temp;
				}
			}
		} else if(jdata.is_object() && jdata["message"].is_string()) {
			ret = errors[1] + jdata["message"].get_to(temp);
			success = false;
			break;
		}

		success = curr_rel != 0 && ret != "";
	} while(0);
	
	curl.slist_free_all(slist);
	curl.easy_cleanup(handle);
	return success;
}

static size_t etag_extract(char *ptr, size_t sz, size_t nmemb, void *ud) {
	size_t out = sz * nmemb;
	if(out > 10 && _strnicmp(ptr, "etag: ", 6) == 0) {
		std::string *str = (std::string *)ud;
		// Вот такой вот странный, но действенный способ обрезать символы \r\n
		*str = std::string{ptr + 6, out - (ptr[out - 2] == '\r' ? 8 : (ptr[out - 1] == '\n' ? 7 : 6))};
	}

	return out;
}

bool UpdatesController::CheckNativeDB(std::string &ret) {
	if(curl.lib == nullptr && !Prepare()) {
		ret = errors[0];
		return false;
	}

	FILE *file = fopen(REDLUA_NATIVES_FILE, "r+");
	if(!file && (file = fopen(REDLUA_NATIVES_FILE, "w")) == NULL) {
		ret = "Failed to open " REDLUA_NATIVES_FILE " file!";
		return false;
	}

	bool success = false;
	std::string etag = "W/\"\"";
	void *slist = default_headers();
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

	do {
		int res;
		if((res = curl.easy_perform(handle)) != 0/*CURLE_OK*/) {
			ret = errors[2] + (std::string)curl.easy_strerror(res);
			break;
		}

		long hcode = 0;
		curl.easy_getinfo(handle, 0x200002/*CURLINFO_RESPONSE_CODE*/, &hcode);

		if(hcode == 200) {
			Settings.Write("nativedb_etag", "W/" + etag);
			success = true;
			break;
		}

		if(hcode == 304)
			ret = "You already have the latest version of NativeDB";
		else
			ret = errors[2] + std::to_string(hcode);
	} while(0);


	curl.slist_free_all(slist);
	curl.easy_cleanup(handle);
	fclose(file);
	return success;
}
