#include <stdbool.h>
#include <string.h>
#include <mem.h>

/* THIS FILE NEEDS TO BE REVISED TO CONFORM TO C99 */

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

/* STANDARD LIBRARY FUNCTIONS */
static char *oldstr;

void *memchr(const void *ptr, int c, size_t num) {
    size_t i;
    for (i = 0; i < num; i++, ptr++)
        if (*((const char *) ptr) == (unsigned char) c)
            return (void *) ptr + i;
    
    return NULL;
}

char *strchr(const char *str, int c) {
    size_t i;
    for (i = 0; i < strlen(str) + 1; i++)
        if (str[i] == (unsigned char) c)
            return (char *) str + i;
    
    return NULL;
}

char *strrchr(const char *str, int c) {
    int i;
    for (i = strlen(str) - 1; i > -1; i--)
        if (str[i] == (unsigned char) c)
            return (char *) str + i;
    
    return NULL;
}

// this function is ripped from GNU GLibC (version 2.22)
size_t strspn(const char *str1, const char *str2) {
    size_t num = 0;
    size_t i;
    size_t j;
    for (i = 0; str1[i] != '\0'; i++) {
        for (j = 0; str2[j] != '\0'; j++)
            if (str1[i] == str2[j])
                break;
            
        if (str2[j] == 0)
            return num;
        else
            ++num;
    }

    return num;
}

size_t strcspn(const char *str1, const char *str2) {
    size_t i;
    size_t j;
    for (i = 0; i < strlen(str1) + 1; i++)
        for (j = 0; j < strlen(str2) + 1; j++) 
            if (str1[i] == str2[j])
                return i;
    
    return strlen(str1);
}

//this one is a little wonky
char *strstr(const char *str1, const char *str2) {
    size_t i;
    for (i = 0; str1[i] != '\0'; i++) {

        size_t j;
        for (j = 0; str1[i + j] != '\0' && str2[j] != '\0'; j++)
            if (str1[i + j] != str2[j])
                break;
        
        if (str2[j] == '\0')
            return (char *) str1 + i;
        else 
            i += j;
    }

    return NULL;
}

// pretty much riped from GNU GlibC (lines 139-142 are different, but not in effect)
char *strtok(char *str, const char *delimeters) {
    char *token;

    if (str == NULL)
        str = oldstr;
    
    str += strspn(str, delimeters);
    if (*str == 0) {
        oldstr = str;
        return NULL;
    }

    token = str;
    str = strpbrk(token, delimeters);
    if (str == NULL) {
        // finish token string
        char *c = token;
        while (*c != '\0')
            c++;
        oldstr = c;
    } else {
        *str = 0;
        oldstr = str + 1;
    }

    return token;
}

// this function is ripped from GNU GLibC (version 2.22)
char *strpbrk(const char *str1, const char *str2) {
    while (*str1 != '\0') {
        const char *s2 = str2;

        while (*s2 != '\0')
            if (*s2++ == *str1)
                return (char *) str1;
        ++str1;
    }

  return NULL;
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

char *strcat(char *dest, const char *src) {
    char *dest_end = dest + strlen(dest); 

    int i;
    for (i = 0; src[i] != '\0'; i++)
        dest_end[i] = src[i];
    
    dest_end[i] = '\0';
    return dest;
}

char *strncat(char *dest, const char *src, size_t num) {
    char *dest_end = dest + strlen(dest);

    size_t i;
    for (i = 0; i < num && *src != '\0'; i++)
        dest_end[i] = src[i];
    
    dest_end[i] = '\0';
    return dest;
}

/* returns the length of a null-terminated string */
size_t strlen(const char *str) {

    int i = 0;
    while (str[i] != 0)
        i++;
    
    return i;
}

/* returns < 0 if s1 < s2, 0 if s1 == s2, > 0 if s1 > s2 */
int strcmp(const char *s1, const char *s2) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') 
            return 0;
    }
    return s1[i] - s2[i];
}

int strncmp(const char *s1, const char *s2, size_t num) {
    size_t i;
    for (i = 0; i < num && s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') 
            return 0;
    }

    return s1[i] - s2[i];
}


char *strcpy(char *dest, const char *src) {
    return memcpy (dest, src, strlen(src) + 1);
}

/* copies num chars from source to destination
   destination and source shouldn't overlap
   if source is less than num, then destination is padded with 0s until num
   chars have been copied */
char *strncpy(char *dest, const char *src, size_t num) {
    size_t i;
    for (i = 0; i < num && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }

    /* pad with 0s until num chars have been copied */
    for (; i < num; i++) {
        dest[i] = '\0';
    }

    return dest;
}

/* appends a character to a string */
void append(char str[], char c) {
    char *tmp = str;
    tmp += strlen(str);

    *tmp = c;
    tmp++;
    *tmp = '\0';
}

/* strcoll can't be properly implemented right now because I don't have a C locale */
int strcoll(const char *str1, const char *str2) {
    return strcmp(str1, str2);
}

/* not implemented for the same reason strcoll can't be properly done */
size_t strxfrm(char *dest __attribute__ ((unused)), const char *src __attribute__ ((unused)), size_t num __attribute__ ((unused))) {
    return 0;
}

char *strerr(int errnum) {
    if (errnum == 0)
        return "No error";
    else
        return "Some error has occurred (This function is not implemented right now)";
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

    while ((size_t) i < strlen(str) && str[i] == ' ')
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

    while ((size_t) i < strlen(str) && str[i] == ' ')
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