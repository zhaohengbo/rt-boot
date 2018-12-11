#include <kernel/rtthread.h>
#include <drivers/rtdevice.h>
#include <dfs/dfs_posix.h>

#include <global/global.h>
#include <board/flash.h>
#include <reboot/reboot.h>

static char buff[128];

void httpd_cgi_handler(char * cgi_file_name)
{
	int fd;
	if(rt_strstr(cgi_file_name, "upgrade_query") != RT_NULL)
	{
		int status = board_flash_get_status();
		int len = 0;
		unlink("/ram/upgrade_query.html");
		fd = open("/ram/upgrade_query.html", O_WRONLY | O_CREAT | O_TRUNC, 0);
		if (fd < 0)
    	{
        	return;
    	}
		if((status >= 0) && (status <= 100))
		{
			len = rt_sprintf(buff,"%d",status);
			write(fd, buff, len);
		}
		else
		{
			write(fd, "SE", rt_strlen("SE"));
		}
		close(fd);
	}
	else if(rt_strstr(cgi_file_name, "board_info") != RT_NULL)
	{
		int len = 0;
		fd = open("/ram/board_info.html", O_WRONLY | O_CREAT | O_TRUNC, 0);
		if (fd < 0)
    	{
        	return;
    	}
		len = rt_sprintf(buff,"%s",rtboot_data.board_name);
		write(fd, buff, len);
		close(fd);
	}
	else if(rt_strstr(cgi_file_name, "soc_info") != RT_NULL)
	{
		int len = 0;
		fd = open("/ram/soc_info.html", O_WRONLY | O_CREAT | O_TRUNC, 0);
		if (fd < 0)
    	{
        	return;
    	}
		len = rt_sprintf(buff,"%s",rtboot_data.soc_name);
		write(fd, buff, len);
		close(fd);
	}
	else if(rt_strstr(cgi_file_name, "eth_info") != RT_NULL)
	{
        int len = 0;
		fd = open("/ram/eth_info.html", O_WRONLY | O_CREAT | O_TRUNC, 0);
		if (fd < 0)
    	{
        	return;
    	}
		len = rt_sprintf(buff,"%s",rtboot_data.eth_name);
		write(fd, buff, len);
		close(fd);
	}
	else if(rt_strstr(cgi_file_name, "ram_info") != RT_NULL)
	{
        int len = 0;
		fd = open("/ram/ram_info.html", O_WRONLY | O_CREAT | O_TRUNC, 0);
		if (fd < 0)
    	{
        	return;
    	}
		len = rt_sprintf(buff,"%s",rtboot_data.ram_size);
		write(fd, buff, len);
		close(fd);
	}
	else if(rt_strstr(cgi_file_name, "flash_info") != RT_NULL)
	{
        int len = 0;
		fd = open("/ram/flash_info.html", O_WRONLY | O_CREAT | O_TRUNC, 0);
		if (fd < 0)
    	{
        	return;
    	}
		len = rt_sprintf(buff,"%s",rtboot_data.flash_info);
		write(fd, buff, len);
		close(fd);
	}
    else if(rt_strstr(cgi_file_name, "clock_info") != RT_NULL)
	{
        int len = 0;
		fd = open("/ram/clock_info.html", O_WRONLY | O_CREAT | O_TRUNC, 0);
		if (fd < 0)
    	{
        	return;
    	}
		len = rt_sprintf(buff,"%s",rtboot_data.clock_info);
		write(fd, buff, len);
		close(fd);
	}
    else if(rt_strstr(cgi_file_name, "version_info") != RT_NULL)
    {
        int len = 0;
		fd = open("/ram/version_info.html", O_WRONLY | O_CREAT | O_TRUNC, 0);
		if (fd < 0)
    	{
        	return;
    	}
		len = rt_sprintf(buff,"%s",rtboot_data.version_info);
		write(fd, buff, len);
		close(fd);
    }
	else if(rt_strstr(cgi_file_name, "reboot") != RT_NULL)
    {
        int len = 0;
        rt_timer_t reboot_timer;
        reboot_timer = rt_timer_create("reboot timer",
                (void *)system_reboot,
                (void *)0,
                RT_TICK_PER_SECOND,
                RT_TIMER_FLAG_ONE_SHOT);
        if (reboot_timer != RT_NULL)
            rt_timer_start(reboot_timer);
        fd = open("/ram/reboot.html", O_WRONLY | O_CREAT | O_TRUNC, 0);
        if (fd < 0)
        {
            return;
        }
        len = rt_sprintf(buff,"<!doctype html>\r\n<html>\r\n<head>\r\n<script tpye=\"text/javascript\">\r\n");
        write(fd, buff, len);
        len = rt_sprintf(buff,"window.opener=null;window.open(\"\",\'_self\');window.close();\r\n");
        write(fd, buff, len);
        len = rt_sprintf(buff,"</script>\r\n</head>\r\n</html>\r\n");
        write(fd, buff, len);
        close(fd);
    }
}
