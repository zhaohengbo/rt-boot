/*
 * File      : wn_sample.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 * This software is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. For the terms of this
 * license, see <http://www.gnu.org/licenses/>.
 *
 * You are free to use this software under the terms of the GNU General
 * Public License, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * Alternatively for commercial application, you can contact us
 * by email <business@rt-thread.com> for commercial license.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-10-26     ChenYong     First version
 */

#include <kernel/rtthread.h>

#include <dfs/dfs_posix.h>
#include <net/webnet/webnet.h>
#include <net/webnet/wn_module.h>

#include <global/global.h>
#include <board/flash.h>
#include <reboot/reboot.h>

//#define HTTPD_DEBUG

#ifdef HTTPD_DEBUG
#define httpd_debug_printf(fmt,args...) rt_kprintf(fmt,##args)
#else
#define httpd_debug_printf(fmt,args...)
#endif

#ifdef WEBNET_USING_UPLOAD

static const char * upload_dir = "/tmp"; /* e.g: "upload" */
static int file_size = 0;

static int file_type = -1;

const char *get_file_name(struct webnet_session *session)
{
    const char *file_name = RT_NULL;
    
    file_name = webnet_upload_get_nameentry(session,"name");
    if (file_name == RT_NULL)
    {
        httpd_debug_printf("file name err!!\n");
        return RT_NULL;
	}
	else
	{
		if(!rt_strcmp(file_name,"fw_file"))
		{
			file_type = 0;
			return "firmware.bin";
		}
		else if(!rt_strcmp(file_name,"uboot_file"))
		{
			file_type = 1;
			return "uboot.bin";
		}
		else if(!rt_strcmp(file_name,"art_file"))
		{
			file_type = 2;
			return "art.bin";
		}
		else if(!rt_strcmp(file_name,"full_file"))
		{
			file_type = 3;
			return "full.bin";
		}
		else
		{
			file_type = -1;
			return RT_NULL;
		}
	}
}

static int upload_open(struct webnet_session *session)
{
    int fd = -1;
    const char *file_name = RT_NULL;
    
    file_name = get_file_name(session);
	
	if(file_name == RT_NULL)
		goto _exit;
	
    httpd_debug_printf("Upload FileName: %s\n", file_name);
    httpd_debug_printf("Content-Type   : %s\n", webnet_upload_get_content_type(session));

    if (webnet_upload_get_filename(session) != RT_NULL)
    {
        int path_size;
        char * file_path;

        path_size = rt_strlen(upload_dir)
                    + rt_strlen(file_name);

        path_size += 4;
        file_path = (char *)rt_malloc(path_size);

        if(file_path == RT_NULL)
        {
            fd = -1;
            goto _exit;
        }

        rt_sprintf(file_path, "%s/%s", upload_dir, file_name);

        httpd_debug_printf("save to: %s\r\n", file_path);

		unlink("/tmp/firmware.bin");
		unlink("/tmp/uboot.bin");
		unlink("/tmp/art.bin");
		unlink("/tmp/full.bin");
        fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0);
        if (fd < 0)
        {
            webnet_session_close(session);
            rt_free(file_path);

            fd = -1;
            goto _exit;
        }
    }

    file_size = 0;

_exit:
    return (int)fd;
}

static int upload_close(struct webnet_session* session)
{
    int fd;

    fd = (int)webnet_upload_get_userdata(session);
    if (fd < 0) return 0;

    close(fd);
	
	switch(file_type)
	{
		case 0:
			board_flash_firmware_notisfy();
			break;
		case 1:
			board_flash_uboot_notisfy();
			break;
		case 2:
			board_flash_art_notisfy();
			break;
		case 3:
			board_flash_full_notisfy();
			break;
		default:
			board_flash_error_notisfy();
			break;
	}
	
    httpd_debug_printf("Upload FileSize: %d\n", file_size);
    return 0;
}

static int upload_write(struct webnet_session* session, const void* data, rt_size_t length)
{
    int fd;

    fd = (int)webnet_upload_get_userdata(session);
    if (fd < 0) return 0;

    httpd_debug_printf("write: length %d\n", length);

    write(fd, data, length);
    file_size += length;

    return length;
}

static int upload_done (struct webnet_session* session)
{
    const char* mimetype;
    static const char* status = "<html><meta http-equiv=\"refresh\" content=\"0;url=/upgrading.html\"></html>\r\n";

    /* get mimetype */
    mimetype = mime_get_type(".html");

    /* set http header */
    session->request->result_code = 200;
    webnet_session_set_header(session, mimetype, 200, "Ok", rt_strlen(status));
    webnet_session_printf(session, status);
	
    return 0;
}

const struct webnet_module_upload_entry upload_entry_upload =
{
    "/upload",
    upload_open,
    upload_close,
    upload_write,
    upload_done
};

#endif /* WEBNET_USING_UPLOAD */

static void cgi_upgrade_query_handler(struct webnet_session* session)
{
    const char* mimetype;
	int status = board_flash_get_status();
    RT_ASSERT(session != RT_NULL);

    /* get mimetype */
    mimetype = mime_get_type(".html");

    /* set http header */
    session->request->result_code = 200;
    webnet_session_set_header(session, mimetype, 200, "Ok", -1);

	if((status >= 0) && (status <= 100))
	{
		webnet_session_printf(session, "%d", status);
	}
	else
	{
		webnet_session_printf(session, "SE");
	} 
}

static void cgi_reboot_handler(struct webnet_session* session)
{
    const char* mimetype;
    RT_ASSERT(session != RT_NULL);

    /* get mimetype */
    mimetype = mime_get_type(".html");

    /* set http header */
    session->request->result_code = 200;
    webnet_session_set_header(session, mimetype, 200, "Ok", -1);
	
	webnet_session_printf(session, "<!doctype html>\r\n<html>\r\n<head>\r\n<script tpye=\"text/javascript\">\r\n");
	webnet_session_printf(session, "window.opener=null;window.open(\"\",\'_self\');window.close();\r\n");
	webnet_session_printf(session, "</script>\r\n</head>\r\n</html>\r\n");
	
	rt_timer_t reboot_timer;
    reboot_timer = rt_timer_create("reboot timer",
            (void *)system_reboot,
            (void *)0,
            RT_TICK_PER_SECOND,
            RT_TIMER_FLAG_ONE_SHOT);
    if (reboot_timer != RT_NULL)
        rt_timer_start(reboot_timer);
}

static void asp_var_soc_info(struct webnet_session* session)
{
    RT_ASSERT(session != RT_NULL);

    webnet_session_printf(session, "%s",rtboot_data.soc_name);
}

static void asp_var_board_info(struct webnet_session* session)
{
    RT_ASSERT(session != RT_NULL);

    webnet_session_printf(session, "%s",rtboot_data.board_name);
}

static void asp_var_flash_info(struct webnet_session* session)
{
    RT_ASSERT(session != RT_NULL);

    webnet_session_printf(session, "%s",rtboot_data.flash_info);
}

static void asp_var_clock_info(struct webnet_session* session)
{
    RT_ASSERT(session != RT_NULL);

    webnet_session_printf(session, "%s",rtboot_data.clock_info);
}

static void asp_var_version_info(struct webnet_session* session)
{
    RT_ASSERT(session != RT_NULL);

    webnet_session_printf(session, "%s",rtboot_data.version_info);
}

static void asp_var_ram_info(struct webnet_session* session)
{
    RT_ASSERT(session != RT_NULL);

    webnet_session_printf(session, "%s",rtboot_data.ram_size);
}

static void asp_var_eth_info(struct webnet_session* session)
{
    RT_ASSERT(session != RT_NULL);

    webnet_session_printf(session, "%s",rtboot_data.eth_name);
}

void httpd_init(void)
{
#ifdef WEBNET_USING_CGI
    webnet_cgi_register("upgrade_query", cgi_upgrade_query_handler);
	webnet_cgi_register("reboot", cgi_reboot_handler);
#endif

#ifdef WEBNET_USING_ASP
    webnet_asp_add_var("soc_info", asp_var_soc_info);
	webnet_asp_add_var("board_info", asp_var_board_info);
	webnet_asp_add_var("eth_info", asp_var_eth_info);
	webnet_asp_add_var("ram_info", asp_var_ram_info);
	webnet_asp_add_var("flash_info", asp_var_flash_info);
	webnet_asp_add_var("clock_info", asp_var_clock_info);
	webnet_asp_add_var("version_info", asp_var_version_info);
#endif

#ifdef WEBNET_USING_ALIAS
    webnet_alias_set("/test", "/admin");
#endif

#ifdef WEBNET_USING_AUTH
    webnet_auth_set("/admin", "admin:admin");
#endif

#ifdef WEBNET_USING_UPLOAD
    webnet_upload_add(&upload_entry_upload);
#endif

    webnet_init();
}
