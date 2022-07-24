#pragma once

#include "thirdparty\scriptmenu.h"
#include <shellapi.h>

class MenuItemLink : public MenuItemDefault {
	string m_link;

	virtual void OnSelect() {
		ShellExecute(0, 0, m_link.c_str(), 0, 0, SW_SHOW);
		SetStatusText("Link will be opened in your default browser in a few moments...");
	}

public:
	MenuItemLink(string caption, string link)
		: MenuItemDefault(caption), m_link(link) {}
};
