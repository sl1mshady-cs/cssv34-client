#include <rmlui_fileinterface.h>
#include <filesystem.h>
#include <tier2/tier2.h>

FileInterface_SrcEng fileinterface;

Rml::FileHandle FileInterface_SrcEng::Open(const Rml::String& path)
{
	FileHandle_t f = g_pFullFileSystem->Open(path.c_str(), "rb", "RMLUI");
	return reinterpret_cast<Rml::FileHandle>(f);
}

void FileInterface_SrcEng::Close(const Rml::FileHandle file)
{
	FileHandle_t f = reinterpret_cast<FileHandle_t>(file);
	g_pFullFileSystem->Close(f);
}

size_t FileInterface_SrcEng::Read(void* buffer, size_t size, Rml::FileHandle file)
{
	FileHandle_t f = reinterpret_cast<FileHandle_t>(file);
	return g_pFullFileSystem->Read(buffer, size, f);
}

bool FileInterface_SrcEng::Seek(Rml::FileHandle file, long offset, int origin)
{
	FileHandle_t f = reinterpret_cast<FileHandle_t>(file);

	g_pFullFileSystem->Seek(f, offset, (FileSystemSeek_t)origin);
	return true;
}

size_t FileInterface_SrcEng::Tell(Rml::FileHandle file)
{
	FileHandle_t f = reinterpret_cast<FileHandle_t>(file);

	return g_pFullFileSystem->Tell(f);
}