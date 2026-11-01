#pragma once
#include "uisystem_internal.h"

int MouseIndexFromCode(ButtonCode_t code);

bool FingerToPixels(const InputEvent_t& event, int& outX, int& outY);