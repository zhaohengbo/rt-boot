#ifndef __ENV__
#define __ENV__

#define ENV_DISABLE 0
#define ENV_ENABLE 1

void env_init(void);
void env_save(void);
void env_set(char *name,char *data);
void env_unset(char *name);
char *env_get(char *name);
void env_list(void);

#endif