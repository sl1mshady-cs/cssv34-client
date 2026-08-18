#include "revCommon.h"
#include "vdf.h"

/*
* INTERNAL; Parse file into a vdfkey
*/
VdfKey* vdf::readFromFile(FILE* file)
{
	char line[1024];

	if (fscanf(file, "%s", line) <= 0) 
		return nullptr;

	if (line[0] != '"' || line[0] == '}') 
		return nullptr;

	// Allocate new key
	VdfKey* key = new VdfKey;
	key->value = 0;
	key->firstChild = 0;
	key->nextSibiling = 0;
	key->name = (char*)malloc(strlen(line) - 1);
	memcpy(key->name, line + 1, strlen(line) - 2);
	key->name[strlen(line) - 2] = 0;

	if (fscanf(file, "%s", line) <= 0)
	{
		::free(key->name);
		delete(key);
		return 0;
	}
	switch (line[0])
	{
	case '"': {
		key->value = (char*)malloc(strlen(line) - 1);
		memcpy(key->value, line + 1, strlen(line) - 2);
		key->value[strlen(line) - 2] = 0;
		return key;
		break;
	}
	case '{': {
		VdfKey* tmp = 0;
		VdfKey* tmp2 = 0;

		while (tmp2 = readFromFile(file))
		{
			if (!tmp)
				key->firstChild = tmp2;
			else
				tmp->nextSibiling = tmp2;
			tmp = tmp2;
		}
		return key;
		break;
	}
	default: {
		::free(key->name);
		delete(key);
		return 0;
	}
	}
	return key;
}


/*
* Parse file into a vdfkey
*/
VdfKey* vdf::parse(char* fileName)
{
	FILE* file = fopen(fileName, "r");
	if (!file)
		return 0;
	VdfKey* res = readFromFile(file);
	fclose(file);
	return res;
}

/*
* Free memory used by vdfkey
*/
void vdf::free(VdfKey* root)
{
	if (root->firstChild) {
		free(root->firstChild);
		delete(root->firstChild);
	}
	if (root->nextSibiling) {
		free(root->nextSibiling);
		delete(root->nextSibiling);
	}

	if (root->name) 
		::free(root->name);

	if (root->value) 
		::free(root->value);
}

/*
* INTERNAL; Save vdfkey to a file
*/
int vdf::saveToFile(FILE* file, VdfKey* key, char* indent)
{
	if (key->value)	
		fprintf(file, "%s\"%s\"\t\t\"%s\"\r\n\n", indent, key->name, key->value);
	else
		if (key->firstChild)
		{
			fprintf(file, "%s\"%s\"\r\n\n", indent, key->name);
			fprintf(file, "%s{\n", indent);
			VdfKey* tmp = key->firstChild;
			strcat(indent, "  ");
			while (tmp)
			{
				saveToFile(file, tmp, indent);
				tmp = tmp->nextSibiling;
			}
			indent[strlen(indent) - 2] = 0;
			fprintf(file, "%s}\n", indent);
		}
	// TODO: return 1 on failure
	return 0;
}

/*
* Save vdfkey to a file
*/
int vdf::save(char* fileName, VdfKey* root)
{
	char indent[100];
	memset(indent, 0, 100);
	FILE* file = fopen(fileName, "w");
	if (!file) // file not found or cannot be opened
		return 1;

	int res = saveToFile(file, root, indent);

	fclose(file); 
	return res;
}