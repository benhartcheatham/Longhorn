#ifndef _STRING_H
#define _STRING_H

/* includes */
#include <stddef.h>
/* defines */

/* structs */

/* typedefs */

/* functions */
char *int_to_string(int n);
char *int_to_hexstring(int n);
void reverse(char *src);
int strlen(const char *str);
int strcmp(char *str1, char *str2);
char *strncpy(char *destination, const char *source, size_t num);
char *to_uppercase(char *str);
char *trim(char *str);
char *strim(char *str, char buffer[]);

#endif