#include "updatesctl.hpp"
#include "constants.hpp"
#include "settingsctl.hpp"
#include "langctl.hpp"

#include "thirdparty\json.hpp"
#include <Windows.h>
#include <WinInet.h>
#include <string>

UpdatesController UpdatesCtl;
using json = nlohmann::json;

static HINTERNET hInternet = NULL;

bool UpdatesController::Prepare(void) {
	if (!hInternet)
		hInternet = InternetOpen(REDLUA_NAME "/" REDLUA_VERSION, INTERNET_OPEN_TYPE_PRECONFIG,
			NULL, NULL, 0);
	return hInternet != NULL;
}

void UpdatesController::Stop(void) {
	if (hInternet) InternetCloseHandle(hInternet);
}

static HINTERNET openRequest(std::string url, std::string headers) {
	return InternetOpenUrl(hInternet, url.c_str(), headers.c_str(), headers.length(),
	INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_PRAGMA_NOCACHE, 0);
}

UpdatesController::Returns UpdatesController::CheckRedLua(std::string &vername) {
	if (!Prepare()) return ERR_WININET_INIT;

	vername = "";
	Returns code = ERR_NO_UPDATES;
	std::string temp;
	static const DWORD BUFSIZE = 1024;
	BYTE buf[BUFSIZE];
	DWORD read = 0;

	HINTERNET hRequest;
	do {
		if ((hRequest = openRequest(REDLUA_TAGS_URL, "Accept: application/json")) == NULL) {
			code = ERR_OPEN_REQUEST;
			break;
		}

		do {
			if (!InternetReadFile(hRequest, buf, BUFSIZE, &read)) {
				code = ERR_READ_RESPONSE;
				break;
			}
			
			temp.append((const char *)buf, read);
		} while (read > 0);
		if (code != ERR_NO_UPDATES) break;

		json jdata = json::parse(temp, nullptr, false);
		if (jdata.is_discarded()) {
			code = ERR_MALFORMED_JSON;
			break;
		}

		int curr_rel = -1;
		if (jdata.is_array()) {
			for (auto &x : jdata.items()) {
				json &jname = x.value()["name"];
				if (jname.is_string()) {
					int vidx = std::strtoul(x.key().c_str(), nullptr, 10);
					if (jname.get_to(temp) == REDLUA_VERSION)
						curr_rel = vidx;
					if (vidx == 0)
						vername = temp;
				}
			}
		} else if (jdata.is_object() && jdata["message"].is_string()) {
			code = ERR_MALFORMED_JSON;
			break;
		}

		if (curr_rel != 0 && vername != "")
			code = OK;
		else if (curr_rel == 0)
			code = ERR_NO_UPDATES;

	} while (0);

	InternetCloseHandle(hRequest);
	return code;
}

UpdatesController::Returns UpdatesController::CheckNativeDB(bool force_update) {
	if (!Prepare()) return ERR_WININET_INIT;

	Returns code = ERR_NO_UPDATES;
	FILE *file = NULL;
	HINTERNET hRequest;

	do {
		static const DWORD BUFSIZE = 1024;
		BYTE buf[BUFSIZE];
		DWORD read = 0;
		std::string etag = "W/\"\"",
		headers = "Accept: application/json";

		if (!force_update)
			headers.append("\r\nIf-None-Match: " + Settings.Read("nativedb_etag", etag));
		if ((hRequest = openRequest(REDLUA_NATIVEDB_URL, headers)) == NULL) {
			code = ERR_OPEN_REQUEST;
			break;
		}

		DWORD status = 0; DWORD statuslen = sizeof(DWORD);
		if (!HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &statuslen, NULL)) {
			code = ERR_QUERY_INFO;
			break;
		}

		if (status == 304)
			break;
		
		if (status != 200) {
			code = ERR_UNKNOWN_RESPONSE;
			break;
		}

		if ((file = fopen(REDLUA_NATIVES_FILE, "w")) == NULL) {
			code = ERR_IO_ISSUE;
			break;
		}

		DWORD bufsize = BUFSIZE;
		if (!HttpQueryInfoA(hRequest, HTTP_QUERY_ETAG, buf, &bufsize, NULL)) {
			code = ERR_QUERY_INFO;
			break;
		}

		etag = std::string{(const char *)buf, bufsize};
		Settings.Write("nativedb_etag", etag);
		int errorcode = 0;
		do {
			if (!InternetReadFile(hRequest, buf, BUFSIZE, &read)) {
				code = ERR_READ_RESPONSE;
				break;
			}

			if (fwrite(buf, 1, read, file) != read) {
				code = ERR_IO_ISSUE;
				break;
			}
		} while (read  > 0);
	} while (0);

	InternetCloseHandle(hRequest);
	if (file) fclose(file);
	return code;
}
