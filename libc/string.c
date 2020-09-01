#include "string.h"

/* converts an int n into an ascii string str */
char *int_to_string(int n) {
    int i = 0;
    static char str[32];

    int num = n;
    do {
        str[i++] = num % 10 + '0';
    } while ((num /= 10) > 0);
    
    str[i] = '\0';

    reverse(str);
    return str;
}

/* converts an int n into an ascii hex string */
char *int_to_hexstring(int n) {
    int i = 0;
    static char str[32];

    int num = n;
    do {
        if (num % 16 >= 10)
            str[i++] = ((num - 10) % 16) + 'a';
        else
            str[i++] = num % 16 + '0';
    } while ((num /= 16) > 0);

    str[i++] = 'x';
    str[i++] = '0';
    str[i] = '\0';

    reverse(str);
    return str;
}

/* reverses a null terminated string */
void reverse(char *src) {
    int i, j;

    for (i = 0, j = strlen(src) - 1; i < j; i++, j--) {
        char temp = src[i];
        src[i] = src[j];
        src[j] = temp;
    }
}

/* returns the length of a null-terminated string */
int strlen(char *str) {
    char *tmp = str;

    int i = 0;
    while (tmp[i] != 0)
        i++;
    
    return i;
}

/* returns < 0 if s1 < s2, 0 if s1 == s2, > 0 if s1 > s2 */
int strcmp(char *s1, char *s2) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    return s1[i] - s2[i];
}

/* appends a character to a string */
void append(char str[], char c) {
    char *tmp = str;
    tmp += strlen(str);

    *tmp = c;
    tmp++;
    *tmp = '\0';
}

/* Creates a all upperscase version of str and puts it into a buffer
   str must be less than 256 characters long*/
char *to_uppercase(char *str) {
    char *curr = str;
    static char up[256];
    char *up_curr = up;

    while (*curr != '\0') {
        if (*curr >= 'a' && *curr <= 'z')
            *up_curr = *curr - 32;
        else
            *up_curr = *curr;
        
        up_curr++;
        curr++;
    }

    return up;
}

/* removes any leading and trailing whitespace
   corrupts memory at str, use strim if this isn't
   desired behaviour */
char *trim(char *str) {
    int i = 0;
    int j = strlen(str) - 1;

    while (i < strlen(str) && str[i] == ' ')
        i++;

    while (j >= 0 && str[j] == ' ')
        j--;

    //j < i is just for safety
    if (j == i || j < i) {
        str[0] = '\0';
        return str;
    }

    str[j + 1] = '\0';
    str = str + i;

    return str;
}

/* removes any leading and trailing whitespace
   and stores the resultant string in buffer */
char *strim(char *str, char buffer[]) {
    int i = 0;
    int j = strlen(str) - 1;

    while (i < strlen(str) && str[i] == ' ')
        i++;
    
    while (j >= 0 && str[j] == ' ')
        j--;
    
    if (j == i || j < i)
        return '\0';
    
    int k;
    for (k = 0; i <= j; k++, i++)
        buffer[k] = str[i];
    
    buffer[k] = '\0';
    return buffer;
}