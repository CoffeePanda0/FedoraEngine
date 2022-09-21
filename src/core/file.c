#include <stdlib.h>
#include <stdio.h>
#include "include/utils.h"
#include "include/strarr.h"
#include "lib/string.h"
#include <stdbool.h>

#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#endif

#ifdef _WIN32
	#include "../ext/dirent.h" // for finding textures in dir
#else
	#include <dirent.h> // for finding textures in dir
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
	uint16_t len = (str) ? mstrlen(str) : 0;
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

FE_StrArr *FE_File_GetFiles(const char *dir)
{
	FE_StrArr *arr = FE_StrArr_Create();
	if (!FE_File_DirectoryExists(dir))
		return arr;
	
	DIR *d = opendir(dir);

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (mstrcmp(ent->d_name, ".") == 0 || mstrcmp(ent->d_name, "..") == 0)
            continue;
        FE_StrArr_Add(arr, ent->d_name);
    }
    closedir(d);

	return arr;
}