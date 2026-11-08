#pragma once
#include "uihudmgr.h"
#include <mathlib/vector.h>
#include <cdll_int.h>
#include <hudmanager/uihudmgr.h>
#include <KeyValues.h>
#include <filesystem.h>
#include <string>


class CHudRadar_Rml
{
public:
	void Initialize();
	void Think();
	void Shutdown();
	void WorldToRadar(const Vector& location, const Vector& origin,
		const QAngle& angles,
		float& x, float& y, float& z_delta);
};