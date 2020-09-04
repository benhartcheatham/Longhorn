#ifndef _STRING_H
#define _STRING_H

/* includes */

/* defines */

/* structs */

/* typedefs */

/* functions */
char *int_to_string(int n);
char *int_to_hexstring(int n);
void reverse(char *src);
int strlen(char *str);
int strcmp(char *str1, char *str2);
char *to_uppercase(char *str);
char *trim(char *str);
char *strim(char *str, char buffer[]);

#endif