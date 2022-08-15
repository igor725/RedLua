#pragma once

#include "langctl.hpp"

#include "thirdparty\scriptmenu.h"
#include <shellapi.h>

class MenuItemLink : public MenuItemDefault {
	std::string m_link;

	void OnSelect() {
		ShellExecute(0, 0, m_link.c_str(), 0, 0, SW_SHOW);
		SetStatusText(Lng.Get("core.nfy.redir"));
	}

public:
	MenuItemLink(std::string caption, std::string link)
	: MenuItemDefault(caption), m_link(link) {}
};

class MenuItemButton : public MenuItemDefault
{
	void (*m_callback)(MenuController *);

	void OnSelect() {
		m_callback(this->GetMenu()->GetController());
	}

public:
	MenuItemButton(std::string caption, void (*callback)(MenuController *))
	: MenuItemDefault(caption), m_callback(callback) {}
};

class MenuTemporary : public MenuBase
{
	void OnPop(void) {
		delete this;
	}

public:
	MenuTemporary(MenuItemTitle *title)
	: MenuBase(title) {}
};
