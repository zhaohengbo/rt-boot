/*
 */

#define LINUX_MAX_ENVS	512
#define LINUX_MAX_ARGS	512

extern image_header_t header;

static int linux_argc;
static int linux_env_idx;

static char **linux_argv;
static char **linux_env;
static char  *linux_env_p;

static void linux_params_init(ulong start, char *cmdline)
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

		while (next != NULL && quote != NULL && quote < next) {
			/*
			 * We found a left quote before the next blank
			 * now we have to find the matching right quote
			 */
			next = strchr(quote + 1, '"');
			if (next != NULL) {
				quote = strchr(next + 1, '"');
				next  = strchr(next + 1, ' ');
			}
		}

		if (next == NULL)
			next = cmdline + strlen(cmdline);

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
		rt_sprintf(memstr, "mem=%luM", gd->ram_size >> 20);
		rt_memcpy(argp, memstr, strlen(memstr) + 1);

		linux_argv[linux_argc] = argp;
		linux_argc++;

		argp += rt_strlen(memstr) + 1;
	}

	linux_env = (char **)(((ulong)argp + 15) & ~15);
	linux_env[0] = 0;

	linux_env_p = (char *)(linux_env + LINUX_MAX_ENVS);
	linux_env_idx = 0;
}

static void linux_env_set(char *env_name, char *env_val)
{
	if (linux_env_idx < LINUX_MAX_ENVS - 1) {
		linux_env[linux_env_idx] = linux_env_p;

		strcpy(linux_env_p, env_name);
		linux_env_p += strlen(env_name);

		strcpy(linux_env_p, "=");
		linux_env_p += 1;

		strcpy(linux_env_p, env_val);
		linux_env_p += strlen(env_val);

		linux_env_p++;
		linux_env[++linux_env_idx] = 0;
	}
}

void arch_linux_loader(void)
{
	int lsdk_kernel;
	char *cmdline, *s, buf[16];
	image_header_t *hdr = &header;
	void(*Kernel)(int, char **, char **);
	void(*LSDKKernel)(int, char **, ulong, ulong);

	//cmdline = getenv("bootargs");

	linux_params_init(UNCACHED_SDRAM(gd->bd->bi_boot_params), cmdline);

	rt_sprintf(buf, "%lu", gd->ram_size >> 20);
	linux_env_set("memsize", buf);

	rt_sprintf(buf, "0x%X", (unsigned int)(gd->bd->bi_flashstart));
	linux_env_set("flash_start", buf);

	rt_sprintf(buf, "0x%X", (unsigned int)(gd->bd->bi_flashsize));
	linux_env_set("flash_size", buf);

	/* We assume that the kernel is in place */
	rt_kprintf("Starting kernel...\n\n");

	//s = getenv("lsdk_kernel");
	//lsdk_kernel = s ? simple_strtol(s, NULL, 10) : 0;
	
	if (lsdk_kernel > 0) {
		LSDKKernel = (void (*)(int, char **,
				       ulong, ulong))ntohl(hdr->ih_ep);

		LSDKKernel(linux_argc, linux_argv, gd->ram_size,
			   gd->bd->bi_flashsize >> 20);
	} else {
		Kernel = (void (*)(int, char **, char **))ntohl(hdr->ih_ep);

		Kernel(linux_argc, linux_argv, linux_env);
	}
}
