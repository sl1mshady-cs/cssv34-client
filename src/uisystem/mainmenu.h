#pragma once
#include "uisystem_internal.h"
#include "uidocument_internal.h"

class MainMenu
{
public:
	void Init();
	void Think();
	void Shutdown();
};

extern MainMenu mainmenu;