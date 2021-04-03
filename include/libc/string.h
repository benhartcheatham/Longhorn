/* Defines standard library string functions */
#ifndef _STRING_H
#define _STRING_H

/* includes */
#include <stddef.h>
/* defines */

/* structs */

/* typedefs */

/* functions */
extern void *memchr(const void *ptr, int c, size_t num);
extern char *strchr(const char *str, int c);
extern char *strrchr(const char *str, int c);
extern size_t strspn(const char *str1, const char *str2);
extern size_t strcspn(const char *str1, const char *str2);
extern char *strstr(const char *str1, const char *str2);
extern char *strtok(char *str, const char*delimeter);
extern char *strpbrk(const char *str1, const char *str2);
extern char *strcat(char *dest, const char *src);
extern char *strncat(char *dest, const char *src, size_t num);
extern char *int_to_string(int n);
extern char *int_to_hexstring(int n);
extern void reverse(char *src);
extern size_t strlen(const char *str);
extern int strcmp(const char *str1, const char *str2);
extern int strncmp(const char *str1, const char *s2, size_t num);
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, size_t num);
extern int strcoll(const char *str1, const char *str2);
extern size_t strxfrm(char *dest, const char *src, size_t num);
extern char *strerr(int errnum);
extern char *to_uppercase(char *str);
extern char *trim(char *str);
extern char *strim(const char *str, char buffer[]);

#endif