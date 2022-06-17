#include <stdlib.h>
#include <stdio.h>
#include "string.h"

#include "../include/utils.h" /* INCLUDE CUSTOM MEMORY ALLOCATOR HERE */

#ifndef assert
#define assert(x) if(!(x)) { printf("String Assertion failed: %s\n", #x); exit(1); }
#endif

char *mstrrand(size_t len)
{
	char *str = _MSTRING_MALLOC(len + 1);
	for (size_t i = 0; i < len; i++)
		str[i] = rand() % 26 + 'a';
	str[len] = '\0';
	return str;
}

char *mstrmult(const char *str, size_t n)
{
	char *out = _MSTRING_MALLOC(sizeof(char) * (mstrlen(str) * n + 1));
	for (size_t idx = 0; idx < n; idx++)
		mstrcat(out, str);
	return out;
}

char *mstrfish(size_t num)
{
	char *ret = mstrmult("  _   \n><_>", num); // fish
	return ret; // return fish
}

size_t mstrcspn(const char *s, const char *reject)
{
	for (size_t i = 0; s[i] != '\0'; i++) {
		for (size_t j = 0; reject[j] != '\0'; j++) {
			if (s[i] == reject[j])
				return i;
		}
	}
	return 0;
}

int mstrcmp(const char *str1, const char *str2)
{
	assert(str1 != NULL && str2 != NULL);
	register const unsigned char *s1 = (const unsigned char*)str1;
	register const unsigned char *s2 = (const unsigned char*)str2;
	
	while (*s1 == *s2 && *s1 != '\0') {s1++; s2++;}
	return (unsigned int)(*s1 > *s2) - (*s1 < *s2);
}

int mstrncmp(const char *str1, const char *str2, size_t n)
{
	assert(str1 != NULL && str2 != NULL);
	register const unsigned char *s1 = (const unsigned char*)str1;
	register const unsigned char *s2 = (const unsigned char*)str2;

	while (*s1 == *s2 && *s1 != '\0' && n > 0) {s1++; s2++; n--;}
	return (unsigned int)(*s1 > *s2) - (*s1 < *s2);
}

int mmemcmp(const void *str1, const void *str2, size_t len)
{
	assert(str1 != NULL && str2 != NULL);
	register const unsigned char *s1 = (const unsigned char*)str1;
	register const unsigned char *s2 = (const unsigned char*)str2;

	for (size_t idx = 0; idx < len; idx++)
		if (*s1++ != *s2++) {return s1[-1] < s2[-1] ? -1 : 1;}
	return 0;
}

void *mmemcpy(void *dest, const void *src, size_t n)
{
	assert(dest != NULL && src != NULL);
	for (size_t i = 0; i < n; i++)
		((char *)dest)[i] = ((char *)src)[i];
	return dest;
}

void *mmemset(void *s, int c, size_t n)
{
	assert(s != NULL);
	for (size_t i = 0; i < n; i++)
		((char *)s)[i] = c;
	return s;
}

void *mmemmove(void *dest, const void *src, size_t n)
{
	assert(dest != NULL && src != NULL);
	for (size_t i = 0; i < n; i++)
		((char *)dest)[i] = ((char *)src)[i];
	return dest;
}

size_t mstrlen(const char *str)
{
    const char *s;
    for (s = str; *s; ++s);
    return (s - str);
}

char *mstrcat(char *dest, const char *src)
{
	assert(dest != NULL && src != NULL);
	char *pt = dest;
	while (*dest) dest++;
	while ((*dest++ = *src++));

	return pt;
}

char *mstrcpy(char *dest, const char *src)
{
	assert(dest != NULL && src != NULL);
    mmemcpy(dest, src, mstrlen(src) + 1);
    return dest;
}

char *mstrncpy(char *dest, const char *src, size_t n)
{
	assert(dest != NULL && src != NULL);
    char *d = dest;
    const char *s = src;
    while (n-- && (*d++ = *s++) != '\0');
    return dest;
}

char *mstrdup(const char *str)
{
	assert(str != NULL);
    char *ret = _MSTRING_MALLOC(sizeof(char) * (mstrlen(str) + 1));
    mstrcpy(ret, str);
    return ret;
}

char *mstrndup(char *str, size_t len)
{
	assert(str != NULL);
	if (len > mstrlen(str)) return mstrdup(str);
	char *res = _MSTRING_CALLOC(len + 1, 1);
	res = mmemcpy(res, str, len);
	res[len] = '\0';
	return res;
}

char *mstrlastchr(const char *s, char c)
{
	assert(s != NULL);
	char *p = (char*)s + mstrlen(s);
	while (p != s)
		if (*--p == c)
			return p;
	return 0;
}

char **mstrwrap(char *str, size_t col_width, size_t *line_count)
{
	assert(str != NULL);
	size_t len = mstrlen(str);

	// Make sure we have a valid string to prevent infinite loop
	if (len < col_width) {
		*line_count = 1;
		char **res = _MSTRING_MALLOC(sizeof(char*));
		res[0] = mstrdup(str);
		return res;
	}

	*line_count = 0;
	char **res = 0;

	size_t sorted_chars = 0;
	while (sorted_chars < len) {
		*line_count = *line_count + 1;
		res = _MSTRING_REALLOC(res, *line_count * sizeof(char*));

		// Remove leading spaces
		char *sp = str + sorted_chars;
		while (*sp == ' ' && sorted_chars < len) {
			sorted_chars++;
			sp++;
		}

		char *s = mstrndup(str + sorted_chars, col_width);

		// check if there is a space in the string. If so, wrap the line at the word
		char *p = mstrlastchr(s, ' ');
		if (p && (size_t)(p - s) > col_width / 1.5) {
			res[*line_count - 1] = mstrndup(s, p - s);
			sorted_chars += mstrlen(res[*line_count - 1]);
			free(s);
			continue;
		}
		sorted_chars += mstrlen(s);
		res[*line_count - 1] = s;
	}
	return res;
}

char *mstrsep(char **sp, char *sep)
{
	assert(sp != NULL && sep != NULL);
	char *p, *s;
	if (sp == NULL || *sp == NULL || **sp == '\0') return(NULL);
	s = *sp;
	p = s + mstrcspn(s, sep);
	if (*p != '\0') *p++ = '\0';
	*sp = p;
	return(s);
}

char *mstradd(const char *str, const char *add)
{
	assert(str != NULL && add != NULL);
	char *newstr = _MSTRING_MALLOC(mstrlen(str) + mstrlen(add) + 1);
	mstrcpy(newstr, str);
	mstrcat(newstr, add);
	return newstr;
}

int mstrinarr(char **arr, size_t n, char *str)
{
	assert(arr != NULL);
	for (size_t i = 0; i < n; i++) {
		if (mstrcmp(arr[i], str) == 0)
			return 1;
	}
	return 0;
}