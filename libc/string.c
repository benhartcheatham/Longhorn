/* Implementation of the standard lib string functions, plus a few extra functions */

/* includes */
#include <stdbool.h>
#include <string.h>
#include <mem.h>

/* defines */

/* globals */
static char *oldstr;

/* functions */

/* THIS FILE NEEDS TO BE REVISED TO CONFORM TO C99 */

/** converts an int n into an ascii string str
 * NOTE: not a stdlib function
 * 
 * @param n: number to convert
 * 
 * @return converted number as a string
 */
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

/** converts an int n into an ascii hex string 
 * NOTE: not a stdlib function
 * 
 * @param n: number to convert
 * 
 * @return converted number as a string
 */
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

/** searches for the first occurrence of the character c in the first n 
 * bytes of the memory pointed to by  ptr
 * 
 * @param ptr: memory to search
 * @param c: character to search for
 * @param num: number of bytes to search
 * 
 * @return address of the first byte that matches c in the region pointed to by ptr, 
 *         NULL if it doesn't exist
 */
void *memchr(const void *ptr, int c, size_t num) {
    size_t i;
    for (i = 0; i < num; i++, ptr++)
        if (*((const char *) ptr) == (unsigned char) c)
            return (void *) ptr + i;
    
    return NULL;
}

/** searches a string for the first occurence of the character c
 * 
 * @param str: string to search
 * @param c: character to search for
 * 
 * @return address of c in str, NULL if non-existent
 */
char *strchr(const char *str, int c) {
    size_t i;
    for (i = 0; i < strlen(str) + 1; i++)
        if (str[i] == (unsigned char) c)
            return (char *) str + i;
    
    return NULL;
}

/** searches a string for the first occurence of the character c
 * this function is supposed to be reentrant, but isn't currently
 * 
 * @param str: string to search
 * @param c: character to search for
 * 
 * @return address of c in str, NULL if non-existent
 */
char *strrchr(const char *str, int c) {
    int i;
    for (i = strlen(str) - 1; i > -1; i--)
        if (str[i] == (unsigned char) c)
            return (char *) str + i;
    
    return NULL;
}

/** calculates the length of the initial segment of str1 which consists of characters in strt2
 * this function is ripped from GNU GLibC (version 2.22)
 * 
 * @param str1: string to search
 * @param str2: string containing characters to match against
 * 
 * @return size of substring, 0 if no such substring exists
 */
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

/** calculates the length of the initial segment of string str1 which consists entirely of 
 * characters not in string str2
 * 
 * @param str1: string to search
 * @param str2: string containing characters to not match against
 * 
 * @return size of substring, 0 if no such substring exists
 */
size_t strcspn(const char *str1, const char *str2) {
    size_t i;
    size_t j;
    for (i = 0; i < strlen(str1) + 1; i++)
        for (j = 0; j < strlen(str2) + 1; j++) 
            if (str1[i] == str2[j])
                return i;
    
    return strlen(str1);
}

/** finds the first occurence of string str2 in string str1
 * 
 * @param str1: string to search
 * @param str2: string to search for in str1
 * 
 * @return address of str2 in str1, NULL if str2 is not in str1
 */
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

/** splits str into tokens on boundaries determined by the characters in delimeters
 * the first call to this function should have str point to a valid string, and
 * subsequent calls should point to NULL until tokenizing is done
 * pretty much ripped from GNU GlibC (lines 139-142 are different, but not in effect)
 * 
 * @param str: string to tokenize
 * @param delimeters: string of characters to split str on
 * 
 * @return token if one exists, NULL if tokenizing is done or no token exists
 */
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

/** finds the first character in the string str1 that matches any character specified in str2
 * this function is ripped from GNU GLibC (version 2.22)
 * 
 * @param str1: string to search
 * @param str2: string of characters to search for in str1
 * 
 * @return first occurence of char in str1 that is in str2, NULL if no chars from str2 are in str1
 */
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

/** reverses a string
 * 
 * @param str: string to reverse
*/
void reverse(char *str) {
    int i, j;

    for (i = 0, j = strlen(str) - 1; i < j; i++, j--) {
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}

/** appends the string at src to the string pointed to by dest
 * 
 * @param dest: string to append to
 * @param src: string to append
 * 
 * @return pointer to dest
 */
char *strcat(char *dest, const char *src) {
    char *dest_end = dest + strlen(dest); 

    int i;
    for (i = 0; src[i] != '\0'; i++)
        dest_end[i] = src[i];
    
    dest_end[i] = '\0';
    return dest;
}

/** appends num characters from the string at src to the string at dest
 * 
 * @param dest: string to append to
 * @param src: string to append
 * @param num: number of characters to append
 * 
 * @return pointer to dest
 */
char *strncat(char *dest, const char *src, size_t num) {
    char *dest_end = dest + strlen(dest);

    size_t i;
    for (i = 0; i < num && *src != '\0'; i++)
        dest_end[i] = src[i];
    
    dest_end[i] = '\0';
    return dest;
}

/** returns the length of a null-terminated string 
 * 
 * @param str: string to calculate the length of
 * 
 * @return number of characters in str
 */
size_t strlen(const char *str) {

    int i = 0;
    while (str[i] != 0)
        i++;
    
    return i;
}

/** compares the string at str1 to the string at str2 
 * 
 * @param str1: first string to compare
 * @param str2: second string ot compare
 * 
 * @return the difference between the first differing character in str1 and str2, 0 if there are none
 */
int strcmp(const char *str1, const char *str2) {
    int i;
    // this should probably make sure we don't read garbage if str1 
    // is longer than str2
    for (i = 0; str1[i] == str2[i]; i++) {
        if (str1[i] == '\0') 
            return 0;
    }
    return str1[i] - str2[i];
}

/** compares the first n characters string at str1 to the string at str2 
 * 
 * @param str1: first string to compare
 * @param str2: second string ot compare
 * @param n: number of characters to compare
 * 
 * @return the difference between the first differing character in the first n characters in
 *         str1 and str2, 0 if there are none
 */
int strncmp(const char *s1, const char *s2, size_t n) {
    size_t i;
    for (i = 0; i < n && s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') 
            return 0;
    }

    return s1[i] - s2[i];
}

/** copies the string at src to the memory at dest
 * dest and src shouldn't overlap
 * 
 * @param dest: memory to copy to
 * @param src: string to copy
 * 
 * @return pointer to dest
 */
char *strcpy(char *dest, const char *src) {
    return memcpy(dest, src, strlen(src) + 1);
}

/** copies num chars from src to dest
 * dest and src shouldn't overlap
 * if src is less than num, then dest is padded with 0s until num
 * chars have been copied 
 * 
 * @param dest: memory to copy to
 * @param src: string to copy
 * @param n: number of characters to copy
 * 
 * @return pointer to dest
 */
char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }

    /* pad with 0s until num chars have been copied */
    for (; i < n; i++) {
        dest[i] = '\0';
    }

    return dest;
}

/** appends character c to string str 
 * 
 * @param str: string to append to
 * @param c: character to append
 */
void append(char *str, char c) {
    char *tmp = str;
    tmp += strlen(str);

    *tmp = c;
    tmp++;
    *tmp = '\0';
}

/** NOTE: UNIMPLEMENTED
 * currently just calls strcmp(str1, str2)
 */
int strcoll(const char *str1, const char *str2) {
    return strcmp(str1, str2);
}

/** NOTE: UNIMPLEMENTED */
size_t strxfrm(char *dest __attribute__ ((unused)), const char *src __attribute__ ((unused)), size_t num __attribute__ ((unused))) {
    return 0;
}

/** NOTE: UNIMPLEMENTED */
char *strerr(int errnum) {
    if (errnum == 0)
        return "No error";
    else
        return "Some error has occurred (This function is not implemented right now)";
}

/** creates an all upperscase version of str and puts it into a buffer
 * str must be less than 256 characters long
 * NOTE: not a std lib function
 * 
 * @param str: string to capitalize
 * 
 * @return pointer to capitalized string
 */
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

/** removes any leading and trailing whitespace
 * corrupts memory at str, use strim if this isn't desired behaviour
 * NOTE: not a std lib function
 * 
 * @param str: string to trim
 *
 * @return pointer to memory at str 
 */
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

/** removes any leading and trailing whitespace
 * and stores the resulting string in buffer
 * 
 * @param str: string to trim
 * @param buf: buffer to store the trimmed string in
 *
 * @return pointer to memory at buf 
 */
char *strim(const char *str, char *buf) {
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
        buf[k] = str[i];
    
    buf[k] = '\0';
    return buf;
}