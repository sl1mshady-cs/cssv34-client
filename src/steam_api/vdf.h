#ifndef _VDH_H_
#define _VDH_H_

#ifdef _WIN32
#pragma once
#endif

/*
* Valve Data Format (KeyValues)
*/
typedef struct VdfKey
{
	char* name;
	char* value;
	VdfKey* nextSibiling;
	VdfKey* firstChild;

} VdfKey;

// VDF NAMESPACE
namespace vdf {
	VdfKey* parse(char* fileName);
	void	free(VdfKey* root);
	int		save(char* fileName, VdfKey* root);

	/* Internal functions */
	VdfKey* readFromFile(FILE* file);
	int		saveToFile(FILE* file, VdfKey* key, char* indent);
}

#endif
