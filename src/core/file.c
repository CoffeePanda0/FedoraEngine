#include <stdlib.h>
#include <stdio.h>
#include "include/utils.h"
#include "lib/string.h"
#include "lib/bool.h"

#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#endif

char *FE_File_ReadStr(FILE *f)
{
	uint16_t len = 0;
	char *str = 0;
	fread(&len, sizeof(uint16_t), 1, f);
	str = xmalloc(len + 1);
	fread(str, sizeof(char), len, f);
	str[len] = '\0';
	return str;
}

bool FE_File_WriteStr(FILE *f, const char *str)
{
	uint16_t len = mstrlen(str);
	fwrite(&len, sizeof(uint16_t), 1, f);
	fwrite(str, sizeof(char), len, f);
	return true;
}

bool FE_File_DirectoryExists(const char *path)
{
	if (access(path, F_OK) != 0)
		return false;
	return true;
}
