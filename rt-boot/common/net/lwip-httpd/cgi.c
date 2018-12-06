#include <kernel/rtthread.h>
#include <drivers/rtdevice.h>
#include <dfs/dfs_posix.h>

#include <board/flash.h>

//Let's just use static string now,it's very easy to convert it to dymanic
void httpd_cgi_handler(char * cgi_file_name)
{
	int fd;
	if(rt_strstr(cgi_file_name, "upgrade_query") != RT_NULL)
	{
		char buff[32];
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
	else if(rt_strstr(cgi_file_name, "cpu_information") != RT_NULL)
	{
		fd = open("/ram/cpu_information.html", O_WRONLY | O_CREAT | O_TRUNC, 0);
		if (fd < 0)
    	{
        	return;
    	}
		write(fd, "AR71XX/AR91XX", rt_strlen("AR71XX/AR91XX"));
		close(fd);
	}
	else if(rt_strstr(cgi_file_name, "cpu_information") != RT_NULL)
	{
		fd = open("/ram/cpu_information.html", O_WRONLY | O_CREAT | O_TRUNC, 0);
		if (fd < 0)
    	{
        	return;
    	}
		write(fd, "64MB", rt_strlen("64MB"));
		close(fd);
	}
	else if(rt_strstr(cgi_file_name, "net_information") != RT_NULL)
	{
        fd = open("/ram/net_information.html", O_WRONLY | O_CREAT | O_TRUNC, 0);
		if (fd < 0)
    	{
        	return;
    	}
        write(fd, "AG71XX", rt_strlen("AG71XX"));
		close(fd);
	}
    else if(rt_strstr(cgi_file_name, "clock_information") != RT_NULL)
	{
        fd = open("/ram/clock_information.html", O_WRONLY | O_CREAT | O_TRUNC, 0);
		if (fd < 0)
    	{
        	return;
    	}
        write(fd, "650MHz", rt_strlen("650MHz"));
		close(fd);
	}
    else if(rt_strstr(cgi_file_name, "version_information") != RT_NULL)
    {
        fd = open("/ram/version_information.html", O_WRONLY | O_CREAT | O_TRUNC, 0);
        if (fd < 0)
        {
            return;
        }
        write(fd, "1.0", rt_strlen("1.0"));
        close(fd);
    }
}
