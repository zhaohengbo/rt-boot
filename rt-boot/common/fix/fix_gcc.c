#include <stdint.h>
void *memcpy(void *dst, const void *src, uint32_t count)
{
    char *tmp = (char *)dst, *s = (char *)src;
    uint32_t len;

    if (tmp <= s || tmp > (s + count))
    {
        while (count--)
            *tmp ++ = *s ++;
    }
    else
    {
        for (len = count; len > 0; len --)
            tmp[len - 1] = s[len - 1];
    }

    return dst;
}

void *memset(void *s, int c, uint32_t count)
{
    char *xs = (char *)s;

    while (count--)
        *xs++ = c;

    return s;
}