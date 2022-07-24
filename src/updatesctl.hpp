#pragma once

#include <string>

class UpdatesController {
	virtual bool Prepare(void);

public:
	UpdatesController () { (void)Prepare(); }
	virtual bool CheckRedLua(std::string &ret);
	virtual bool CheckNativeDB(std::string &ret);
};

extern UpdatesController UpdatesCtl;
