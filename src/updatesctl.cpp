#include "updatesctl.hpp"
#include "constants.hpp"
#include "settingsctl.hpp"

#include "thirdparty\json.hpp"
#include <Windows.h>
#include <WinInet.h>
#include <string>

#pragma comment(lib, "delayimp")
#pragma comment(lib, "WinInet")

UpdatesController UpdatesCtl;
using json = nlohmann::json;

static const std::string errors[] = {
	"Internal error: ",
	"Server responded with error: ",
	"HTTP request failed: ",
	"Malformed server response",

	// Internal errors
	"failed to initialize WinInet",
	"InternetOpenUrl failed",
	"HttpQueryInfo failed"
};

static HINTERNET hInternet = NULL;

bool UpdatesController::Prepare(void) {
	if(!hInternet)
		hInternet = InternetOpen("RL/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	return hInternet != NULL;
}

void UpdatesController::Stop(void) {
	if(hInternet) InternetCloseHandle(hInternet);
}

static HINTERNET open_request(std::string url, std::string headers) {
	return InternetOpenUrl(hInternet, url.c_str(), headers.c_str(), headers.length(),
	INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_UI | INTERNET_FLAG_NO_COOKIES, 0);
}

bool UpdatesController::CheckRedLua(std::string &ret) {
	if(!Prepare()) {
		ret = errors[0] + errors[4];
		return false;
	}

	bool success;
	std::string temp;
	static const DWORD BUFSIZE = 1024;
	BYTE buf[BUFSIZE];
	DWORD read = 0;

	HINTERNET hRequest;
	do {
		hRequest = open_request(REDLUA_TAGS_URL, "Accept: application/json");
		if(hRequest == NULL) break;

		while((success = InternetReadFile(hRequest, buf, BUFSIZE, &read)) && read > 0)
			temp.append((const char *)buf, read);
		if(!success) {
			ret = errors[2] + std::to_string(GetLastError());
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

	InternetCloseHandle(hRequest);
	return success;
}

bool UpdatesController::CheckNativeDB(std::string &ret, bool force_update) {
	if(!Prepare()) {
		ret = errors[0] + errors[4];
		return false;
	}

	std::string temp;
	bool success = false;
	static const DWORD BUFSIZE = 1024;
	BYTE buf[BUFSIZE];
	DWORD read = 0;

	FILE *file = NULL;
	HINTERNET hRequest;

	do {
		std::string etag = "W/\"\"",
		headers = "Accept: application/json";
		if(!force_update)
			headers.append("\r\nIf-None-Match: " + Settings.Read("nativedb_etag", etag));
		if((hRequest = open_request(REDLUA_NATIVEDB_URL, headers)) == NULL) {
			ret = errors[0] + errors[5];
			break;
		}

		DWORD code = 0; DWORD codelen = sizeof(DWORD);
		if(!HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &code, &codelen, NULL)) {
			ret = errors[0] + errors[6];
			break;
		}

		if(code == 304) {
			ret = "You already have the latest version of NativeDB";
			break;
		} else if(code != 200) {
			ret = errors[2] + std::to_string(code);
			break;
		}

		if((file = fopen(REDLUA_NATIVES_FILE, "w")) == NULL) {
			ret = "Failed to open " REDLUA_NATIVES_FILE " file: " + (std::string)strerror(errno);
			break;
		}

		DWORD bufsize = BUFSIZE;
		if(!HttpQueryInfoA(hRequest, HTTP_QUERY_ETAG, buf, &bufsize, NULL)) {
			ret = errors[0] + errors[6];
			break;
		}

		etag = std::string{(const char *)buf, bufsize};
		Settings.Write("nativedb_etag", etag);
		int errorcode = 0;
		while((success = InternetReadFile(hRequest, buf, BUFSIZE, &read)) && read > 0)
			if((success = (fwrite(buf, 1, read, file) == read)) == false) {
				errorcode = errno;
				break;
			}
		if(!success)
			ret = errors[2] + std::to_string(errorcode != 0 ? errorcode : GetLastError());
	} while(0);

	InternetCloseHandle(hRequest);
	if(file) fclose(file);
	return success;
}
