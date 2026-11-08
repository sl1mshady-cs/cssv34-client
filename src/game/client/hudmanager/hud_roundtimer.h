#pragma once
#include "uihudmgr.h"
#include <mathlib/vector.h>
#include <cdll_int.h>
#include <hudmanager/uihudmgr.h>
#include <KeyValues.h>
#include <filesystem.h>
#include <string>

class CHudRT
{
public:
	void Initialize();
	void Think();
	void Shutdown();
};