#include <kernel/rtthread.h>
#include <board/flash.h>

#include <env/env.h>

typedef	struct environment_s {
	struct environment_s	*next;
	char					data[];
} env_t;

typedef	struct global_env_s {
	rt_uint32_t		flag;
	rt_uint32_t		length;
	rt_uint32_t		free;
	rt_uint32_t		count;
	env_t			*head;
} env_ctb;

env_ctb global_env_ctb;

static inline rt_int32_t env_is_end(rt_uint8_t * ptr)
{
	if((ptr[0] == 0) && (ptr[1] == 0))
		return 1;
	else
		return 0;
}

static void env_node_add(env_t *new_node)
{
	env_t *node = global_env_ctb.head;
	
	if(node == RT_NULL)
	{
		global_env_ctb.head = new_node;
	}
	else
	{
		while(node->next)
		{
			node = node->next;
		}
		node->next = new_node;
	}
	global_env_ctb.free -= rt_strlen(new_node->data) + 1;
	global_env_ctb.count++;
}

static void env_node_delete(env_t *del_node)
{
	env_t *node = global_env_ctb.head;
	
	if(node == RT_NULL)
		return;
	
	if(node == del_node)
	{
		global_env_ctb.head = del_node->next;
	}
	else
	{
		while((node->next != del_node) && (node->next))
		{
			node = node->next;
		}
		if(node->next)
			node->next = del_node->next;
		else
			return;
	}
	global_env_ctb.free += rt_strlen(del_node->data) + 1;
	global_env_ctb.count--;
}

static env_t * env_node_search(char *name)
{
	env_t *node = global_env_ctb.head;
	while(node)
	{
		int name_len = rt_strlen(name);
		if(node->data[name_len] == '=')
		{
			if(!rt_memcmp(node->data,name,name_len))
			{
				return node;
			}
		}
		
		node = node->next;
	}
	return RT_NULL;
}

void env_init(void)
{
	rt_uint8_t * tmp_buf;
	rt_uint8_t * ptr,* end_ptr;
	
	global_env_ctb.flag = board_get_env_flag();
	
	if(global_env_ctb.flag == ENV_ENABLE)
	{
		global_env_ctb.length = board_get_env_length();
	}
	
	if(global_env_ctb.length)
		tmp_buf = rt_calloc(global_env_ctb.length,1);
	else
		return;
	
	if(tmp_buf)
	{
		board_get_env_read(tmp_buf);
		
		if((tmp_buf[0] != 'E') || (tmp_buf[1] != 'N') || (tmp_buf[2] != 'V') || (tmp_buf[3] != '\0'))
		{
			rt_kprintf("ENV data header error,use default!\n");
			tmp_buf[0] = 'E';
			tmp_buf[1] = 'N';
			tmp_buf[2] = 'V';
			tmp_buf[3] = '\0';
			global_env_ctb.free = global_env_ctb.length - 4;
			return;
		}
		ptr = &tmp_buf[3];
		end_ptr = tmp_buf + global_env_ctb.length;
		
		global_env_ctb.free = global_env_ctb.length - 4;
		
		while(ptr < end_ptr)
		{
			env_t *node;
			if(env_is_end(ptr))
				break;
			
			ptr++;
			
			node = (env_t *)rt_calloc(sizeof(env_t) + rt_strlen((char *)ptr) + 1,1);
			
			if(node == RT_NULL)
			{
				rt_kprintf("ENV no memory!\n");
				return;
			}
			else
			{
				rt_strcpy(node->data,(char *)ptr);
				env_node_add(node);
				ptr += rt_strlen((char *)ptr);
			}
		}
		
		rt_free(tmp_buf);
	}
}

void env_save(void)
{
	rt_uint8_t * tmp_buf;
	rt_uint8_t * ptr,* end_ptr;
	
	if(global_env_ctb.length)
		tmp_buf = rt_calloc(global_env_ctb.length,1);
	else
		return;
	
	if(tmp_buf)
	{
		env_t *node = global_env_ctb.head;
		
		tmp_buf[0] = 'E';
		tmp_buf[1] = 'N';
		tmp_buf[2] = 'V';
		tmp_buf[3] = '\0';

		ptr = &tmp_buf[4];
		end_ptr = tmp_buf + global_env_ctb.length;
		
		while(1)
		{
			if(node)
			{
				int tmp_len = rt_strlen(node->data);
				if(ptr + tmp_len < end_ptr)
				{
					rt_strcpy((char *)ptr,node->data);
					ptr += tmp_len + 1;
					node = node->next;
				}
				else 
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		
		board_get_env_write(tmp_buf);
		rt_free(tmp_buf);
	}
}

void env_set(char *name,char *data)
{
	env_t *node = env_node_search(name);
	
	if(node)
	{
		env_node_delete(node);
		rt_free((void *)node);
	}
	
	node = (env_t *)rt_calloc(sizeof(env_t) + rt_strlen(name) + rt_strlen(data) + 2,1);
			
	if(node == RT_NULL)
	{
		rt_kprintf("ENV no memory!\n");
		return;
	}
	else
	{
		rt_strcat(node->data,name);
		rt_strcat(node->data,"=");
		rt_strcat(node->data,data);
		env_node_add(node);
	}
}

void env_unset(char *name)
{
	env_t *node = env_node_search(name);
	
	if(node)
	{
		env_node_delete(node);
		rt_free((void *)node);
	}
}

char *env_get(char *name)
{
	env_t *node = env_node_search(name);
	
	if(node)
	{
		return &(node->data[rt_strlen(name) + 1]);
	}
	
	return RT_NULL;
}

void env_list(void)
{
	env_t *node = global_env_ctb.head;
	
	rt_kprintf("Current ENV size : %d \n",global_env_ctb.length);
	rt_kprintf("Current ENV free : %d \n",global_env_ctb.free);
	rt_kprintf("Current ENV count : %d \n",global_env_ctb.count);
	rt_kprintf("Current ENV list :\n");
	
	while(node)
	{
		rt_kprintf("%s\n",node->data);
		node = node->next;
	}
}

static void env(uint8_t argc, char **argv) 
{
	if (argc < 2) 
	{
		goto env_too_few_arg;
	}
	else
	{
		const char *operator = argv[1];
		
		if (!rt_strcmp(operator, "set"))
		{
			if (argc < 4)
			{
				goto env_too_few_arg;
			}				
			env_set(argv[2],argv[3]);
		}
		else if (!rt_strcmp(operator, "get"))
		{
			char * data;
			if (argc < 3)
			{
				goto env_too_few_arg;
			}				
			data = env_get(argv[2]);
			if(data != RT_NULL)
				rt_kprintf("%s=%s\n",argv[2],data);
			else
				rt_kprintf("value %s not found\n",argv[2]);
		}
		else if (!rt_strcmp(operator, "unset"))
		{
			if (argc < 3)
			{
				goto env_too_few_arg;
			}				
			env_unset(argv[2]);
		}
		else if (!rt_strcmp(operator, "list"))
		{
			env_list();
		}
		else if (!rt_strcmp(operator, "save"))
		{
			env_save();
		}
		else
		{
			goto env_valid_cmd;
		}
	}
	
	return;

env_valid_cmd:
	rt_kprintf("Valid input cmd!\n");
	goto env_reprint;	
env_too_few_arg:
	rt_kprintf("Too few arg!\n");
	goto env_reprint;
env_reprint:
	rt_kprintf("Try env get <name>\n");
	rt_kprintf("Try env set <name> <data>\n");
	rt_kprintf("Try env unset <name>\n");
	rt_kprintf("Try env list\n");
	rt_kprintf("Try env save\n");
}

MSH_CMD_EXPORT(env, Environment operation.);