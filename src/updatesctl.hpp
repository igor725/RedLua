#pragma once

#include <string>

class UpdatesController {
	bool Prepare(void);

public:
	bool CheckRedLua(std::string &ret);
	bool CheckNativeDB(std::string &ret, bool force_update = false);
	void Stop(void);
};

extern UpdatesController UpdatesCtl;
