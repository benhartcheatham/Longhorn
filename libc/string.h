#ifndef _STRING_H
#define _STRING_H

/* includes */
#include <stddef.h>
/* defines */

/* structs */

/* typedefs */

/* functions */
void *memchr(const void *ptr, int c, size_t num);
char *strchr(const char *str, int c);
char *strrchr(const char *str, int c);
size_t strspn(const char *str1, const char *str2);
size_t strcspn(const char *str1, const char *str2);
char *strstr(const char *str1, const char *str2);
char *strtok(char *str, const char*delimeter);
char *strpbrk(const char *str1, const char *str2);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t num);
char *int_to_string(int n);
char *int_to_hexstring(int n);
void reverse(char *src);
size_t strlen(const char *str);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *s2, size_t num);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t num);
int strcoll(const char *str1, const char *str2);
size_t strxfrm(char *dest, const char *src, size_t num);
char *strerr(int errnum);
char *to_uppercase(char *str);
char *trim(char *str);
char *strim(char *str, char buffer[]);

#endif