#ifndef RMLUI_FILEINTERFACE_H
#define RMLUI_FILEINTERFACE_H

#if defined(_WIN32)
#pragma once
#endif

#include <RmlUi/Core.h>

class FileInterface_SrcEng : public Rml::FileInterface
{
public:
	Rml::FileHandle Open(const Rml::String& path) override;
	void Close(Rml::FileHandle file) override;
	size_t Read(void* buffer, size_t size, Rml::FileHandle file) override;
	bool Seek(Rml::FileHandle file, long offset, int origin) override;
	size_t Tell(Rml::FileHandle file) override;
};

extern FileInterface_SrcEng fileinterface;

#endif