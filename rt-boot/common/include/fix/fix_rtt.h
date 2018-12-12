#ifndef __FIX_RTT__
#define __FIX_RTT__

#define ZEROPAD 	(1 << 0)	/* pad with zero */
#define SIGN 		(1 << 1)	/* unsigned/signed long */
#define PLUS 		(1 << 2)	/* show plus */
#define SPACE 		(1 << 3)	/* space if plus */
#define LEFT 		(1 << 4)	/* left justified */
#define SPECIAL 	(1 << 5)	/* 0x */
#define LARGE		(1 << 6)	/* use 'ABCDEF' instead of 'abcdef' */

#define _U	0x01	/* upper */
#define _L	0x02	/* lower */
#define _D	0x04	/* digit */
#define _C	0x08	/* cntrl */
#define _P	0x10	/* punct */
#define _S	0x20	/* white space (space/lf/tab) */
#define _X	0x40	/* hex digit */
#define _SP	0x80	/* hard space (0x20) */

int rt_isprint(int c);
int rt_isalpha (int c);
int rt_isdigit (int ch);
int rt_isspace(int ch);

int rt_atoi(const char *nptr);
long int rt_atol(const char *nptr);

int rt_rand(void);
int rt_rand_r(unsigned int *seed);
void rt_srand(unsigned int seed);

void rt_qsort(void *base, rt_size_t nmemb, rt_size_t size, int (*compar)(const void *, const void *));

int rt_tolower(int c);
int rt_toupper(int c);
int rt_isxdigit(int c);
int rt_isupper(int c);

int rt_strncasecmp(const char *cs, const char *ct, rt_size_t count);
char *rt_strcpy(char *dest, const char *src);
rt_size_t rt_strlcpy(char *dst, const char *src, rt_size_t siz);
char *rt_strncat(char *dest, const char *src, rt_size_t count);
char *rt_strcat(char * dest, const char * src);
char *rt_strchr(const char *s1, int i);
char *rt_strrchr(const char *t, int c);

char *rt_strtok(char *s, const char *delim);
char *rt_strtok_r(char*s, const char*delim, char**ptrptr);

rt_size_t rt_strcspn(const char *s, const char *reject);
rt_size_t rt_strspn (const char *s, const char *accept);

long rt_strtol(const char *str, char **endptr, int base);
long long rt_strtoll(const char *str, char **endptr, int base);

#endif