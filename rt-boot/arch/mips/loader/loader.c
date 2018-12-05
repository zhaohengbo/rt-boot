#include <kernel/rtthread.h>
#include <global/global.h>
#include <loader/image.h>

#include <arch/cache.h>

#define LINUX_MAX_ENVS	512
#define LINUX_MAX_ARGS	512

static int linux_argc;
static int linux_env_idx;

static char **linux_argv;
static char **linux_env;
static char  *linux_env_p;

static void(*Kernel)(int, rt_uint32_t, rt_uint32_t, rt_uint32_t);

static void linux_params_init(rt_uint32_t start, char *cmdline)
{
	char memstr[16];
	char *next, *quote, *argp;

	linux_argc = 1;
	linux_argv = (char **)start;
	linux_argv[0] = 0;
	argp = (char *)(linux_argv + LINUX_MAX_ARGS);

	next = cmdline;

	if (rt_strstr(cmdline, "mem="))
		memstr[0] = 0;
	else
		memstr[0] = 1;

	while (cmdline && *cmdline && linux_argc < LINUX_MAX_ARGS) {
		quote = rt_strchr(cmdline, '"');
		next  = rt_strchr(cmdline, ' ');

		while (next != RT_NULL && quote != RT_NULL && quote < next) {
			/*
			 * We found a left quote before the next blank
			 * now we have to find the matching right quote
			 */
            next = rt_strchr(quote + 1, '"');
			if (next != RT_NULL) {
                quote = rt_strchr(next + 1, '"');
                next  = rt_strchr(next + 1, ' ');
			}
		}

		if (next == RT_NULL)
            next = cmdline + rt_strlen(cmdline);

		linux_argv[linux_argc] = argp;
		rt_memcpy(argp, cmdline, next - cmdline);
		argp[next - cmdline] = 0;

		argp += next - cmdline + 1;
		linux_argc++;

		if (*next)
			next++;

		cmdline = next;
	}

	/* Add mem size to command line if it's missing' */
	if (memstr[0]) {
        rt_sprintf(memstr, "mem=%luM", rtboot_data.system_memsize >> 20);
        rt_memcpy(argp, memstr, rt_strlen(memstr) + 1);

		linux_argv[linux_argc] = argp;
		linux_argc++;

		argp += rt_strlen(memstr) + 1;
	}

	linux_env = (char **)(((rt_uint32_t)argp + 15) & ~15);
	linux_env[0] = 0;

	linux_env_p = (char *)(linux_env + LINUX_MAX_ENVS);
	linux_env_idx = 0;
}

static void linux_env_set(char *env_name, char *env_val)
{
	if (linux_env_idx < LINUX_MAX_ENVS - 1) {
		linux_env[linux_env_idx] = linux_env_p;

		rt_strcpy(linux_env_p, env_name);
		linux_env_p += rt_strlen(env_name);

		rt_strcpy(linux_env_p, "=");
		linux_env_p += 1;

		rt_strcpy(linux_env_p, env_val);
		linux_env_p += rt_strlen(env_val);

		linux_env_p++;
		linux_env[++linux_env_idx] = 0;
	}
}

void arch_linux_loader(image_header_t *hdr, char *cmdline)
{
    char buf[16];

    linux_params_init(rtboot_data.boot_param_pointer, cmdline);

	rt_sprintf(buf, "%lu", rtboot_data.system_memsize >> 20);
	linux_env_set("memsize", buf);

	//rt_sprintf(buf, "0x%X", (unsigned int)(gd->bd->bi_flashstart));
	//linux_env_set("flash_start", buf);

	//rt_sprintf(buf, "0x%X", (unsigned int)(gd->bd->bi_flashsize));
	//linux_env_set("flash_size", buf);

    arch_cache_flush(rtboot_data.boot_param_pointer, CFG_BOOTPARAMS_LEN);

    Kernel = (void (*)(int, rt_uint32_t, rt_uint32_t, rt_uint32_t))imtohl(hdr->ih_ep);
}

void arch_linux_jump(rt_uint32_t linux_extra)
{
    Kernel(linux_argc, (rt_uint32_t)linux_argv, (rt_uint32_t)linux_env, linux_extra);
}
