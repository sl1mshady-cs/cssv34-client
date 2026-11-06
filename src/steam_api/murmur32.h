#ifndef MURMUR32_H
#define MURMUR32_H

#ifdef _WIN32
#pragma once
#endif

#include "revCommon.h"

uint32 murmur3_32(const byte* key, size_t len, uint32 seed);

#endif // MURMUR32_H
