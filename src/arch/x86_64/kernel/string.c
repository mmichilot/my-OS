#include <stddef.h>
#include <stdint-gcc.h>

void *memset(void *s, int c, size_t n)
{
    size_t i;
    
    for (i = 0; i < n; i++)
        ((uint8_t*)s)[i]= (uint8_t) c;

    return s;
}

void *memcpy(void *dest, const void *src, size_t n) 
{
    size_t i;

    for (i = 0; i < n; i++)
        ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];

    return dest;
}
size_t strlen(const char *s) 
{
    size_t len;

    for (len = 0; s[len] != '\0'; len++);

    return len;
}

char *strcpy(char *dest, const char *src) 
{
    size_t i;

    for (i = 0; src[i] != '\0'; i++)
        dest[i] = src[i];
        
    dest[i] = '\0';

    return dest;
}

int strcmp(const char *s1, const char *s2) 
{
    size_t i;

    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0')
            return 0;
    }
    
    return s1[i] - s2[i];
}

const char *strchr(const char *s, int c)
{
    for (; *s != (char) c; s++) {
        if (*s == '\0')
            return NULL;
    }

    return s;
}