#pragma once

#include <string>

class UpdatesController {
	bool Prepare(void);

public:
	enum Returns {
		OK,
		ERR_WININET_INIT,
		ERR_OPEN_REQUEST,
		ERR_QUERY_INFO,
		ERR_UNKNOWN_RESPONSE,
		ERR_IO_ISSUE,
		ERR_READ_RESPONSE,
		ERR_MALFORMED_JSON,
		ERR_NO_UPDATES
	};

	UpdatesController::Returns CheckRedLua(std::string &vername);
	UpdatesController::Returns CheckNativeDB(bool force_update = false);
	void Stop(void);
};

extern UpdatesController UpdatesCtl;
