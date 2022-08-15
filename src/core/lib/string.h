#ifndef _MSTRING_H
#define _MSTRING_H 1

#include <stddef.h>

#ifndef _MSTRING_ALLOCATOR
#define _MSTRING_ALLOCATOR 1

/* Define custom memory allocator here */
#define _MSTRING_MALLOC xmalloc
#define _MSTRING_REALLOC xrealloc
#define _MSTRING_CALLOC xcalloc

#endif

/**
 * @brief Compares two strings
 * @param str1 The first string to compare
 * @param str2 The second string to compare
 * @return 0 if strings are equal, or otherwise depending on whether str1 <> str2
 */
int mstrcmp(const char *str1, const char *str2);


/**
 * @brief Compares N bytes of strings
 * @param str1 The first string to compare
 * @param str2 The second string to compare
 * @param n The number of bytes to compare
 * @return 0 if strings are equal, or otherwise depending on whether str1 <> str2
 */
int mstrncmp(const char *str1, const char *str2, size_t n);


/**
 * @brief Compares two blocks of memory
 * @param str1 The first block to compare
 * @param str2 
 * @param len 
 * @return 0 if strings are equal, or otherwise depending on whether str1 <> str2
 */
int mmemcmp(const void *str1, const void *str2, size_t len);


/**
 * @brief Copies n amount of memory from src to dst
 * @param dest The destination for the memory to go
 * @param src The source for memory to be copied from
 * @param n The amount of bytes to copy
 * @return A pointer to the dest
 */
void *mmemcpy(void *dest, const void *src, size_t n);


/**
 * @brief Sets N bytes of S to C
 * @param s The block to set
 * @param c The character to set to
 * @param n The number of bytes to set
 * @return void* 
 */
void *mmemset(void *s, int c, size_t n);


/** 
 * @brief Moves src blocks of memory by n to dest
 * @param dest The destination for the memory to go
 * @param src The source for memory to be moved from
 * @param n The amount of bytes to move
 * @return A pointer to the dest
 */
void *mmemmove(void *dest, const void *src, size_t n);


/**
 * @brief Calculates the length of a null-terminated string
 * @param str The string to measure
 * @return The length of the string
 */
size_t mstrlen(const char *str);


/**
 * @brief Adds string src to the end of dest
 * @param dest The destination string
 * @param src The source string
 * @return A pointer to the destination string
 */
char *mstrcat(char *dest, const char *src);


/**
 * @brief Copies dst string to src
 * @param dest The destination string
 * @param src The source to copy
 * @return The destination string
 */
char *mstrcpy(char *dest, const char *src);


/**
 * @brief Copies n bytes of str to dest
 * @param dest The destination string
 * @param src The source string
 * @param n The number of bytes to copy
 * @return The destination string
 */
char *mstrncpy(char *dest, const char *src, size_t n);


/**
 * @brief Duplicates str
 * @param str The string to duplicate
 * @return A malloc'd string identical to str
 */
char *mstrdup(const char *str);


/**
 * @brief Duplicates len bytes of str
 * @param str The string to duplicate
 * @param len The number of bytes to duplicate
 * @return A malloc'd string
 */
char *mstrndup(char *str, size_t len);


/**
 * @brief Returns the string after the final occurance of c
 * @param s The string to check
 * @param c The character to find
 * @return The string past the last char c
 */
char *mstrlastchr(const char *s, char c);


/**
 * @brief Wraps a string by column width, using spaces or wrapping mid-word
 * @param str The string to wrap
 * @param col_width The maximum number of characters for each column
 * @param line_count A number to output the num lines that have been returned
 * @return An array containing each line as a string
 */
char **mstrwrap(char *str, size_t col_width, size_t *line_count);


/**
 * @brief Separates a string
 * @param sp The string to separate
 * @param sep The delimiter
 * @return The separated string
 */
char *mstrsep(char **sp, char *sep);


/**
 * @brief Adds one string to the end of another. Identical to strcat, but also mallocs
 * @param str The source string
 * @param add The string to add to the end of str
 * @return A malloc'd string
 */
char *mstradd(const char *str, const char *add);


/**
 * @brief Detects whether a string exists in an array of strings
 * @param arr The array to search
 * @param n The number of strings in the array to search
 * @param str The string to find
 * @return 0 if string is not found, 1 if string is found
 */
int mstrinarr(char **arr, size_t n, char *str);


/**
 * @brief Calculates the length of s excluding the accepted characters
 * @param s The string to calculate the length of
 * @param accept The characters to include in the length
 * @return The length of s
 */
size_t mstrcspn(const char *s, const char *reject);


/**
 * @brief fish
 * @param num the number of fish
 * @return fish 
 */
char *mstrfish(size_t num);


/** 
 * @brief Multiplies a string by a number
 * @param str The string to output
 * @param n The number to multiply str by
 * @return A malloc'd string containing str n amount of times 
 */
char *mstrmult(const char *str, size_t n);


/** 
 * @brief Creates a string of random characters
 * @param len The length of the string to create
 * @return A malloc'd string of random characters
 */
char *mstrrand(size_t len);

// return true or false if the two strings match
#define MATCH(s, n) mstrcmp(section, s) == 0 && mstrcmp(name, n) == 0

#endif