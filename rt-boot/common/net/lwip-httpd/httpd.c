#include <kernel/rtthread.h>
#include <drivers/rtdevice.h>

#include <net/lwip/lwip/sockets.h>

#include <net/lwip-httpd/httpd.h>
#include <net/lwip-httpd/fs.h>
#include <net/lwip-httpd/fsdata.h>

#define HTTPD_DEBUG

#ifdef HTTPD_DEBUG
#define PRINT(x) rt_kprintf("%s", x)
#define PRINTLN(x) rt_kprintf("%s\n", x)
#else /* HTTPD_DEBUG */
#define PRINT(x)
#define PRINTLN(x)
#endif /* HTTPD_DEBUG */

extern const struct fsdata_file file_index_htm;
extern const struct fsdata_file file_404_htm;

// ASCII characters
#define ISO_G					0x47	// GET
#define ISO_E					0x45
#define ISO_T					0x54
#define ISO_P					0x50	// POST
#define ISO_O					0x4f
#define ISO_S					0x53
#define ISO_T					0x54
#define ISO_slash				0x2f	// control and other characters
#define ISO_space				0x20
#define ISO_nl					0x0a
#define ISO_cr					0x0d
#define ISO_tab					0x09

#define HTTP_PORT 80

#define RECV_BUF_LEN 1600
static rt_uint8_t recv_buf[RECV_BUF_LEN];
static rt_int32_t recv_len = 0;

static char eol[3] = { 0x0d, 0x0a, 0x00 };
static char eol2[5] = { 0x0d, 0x0a, 0x0d, 0x0a, 0x00 };

ALIGN(RT_ALIGN_SIZE)
static char httpd_thread_stack[0x1000];
struct rt_thread httpd_thread;

static void httpd_thread_entry(void* parameter)
{
	struct sockaddr_in addr;
    socklen_t addr_size;
	rt_uint32_t server_fd;
	rt_uint32_t client_fd;
	rt_int32_t i;
	struct fs_file fsfile;
	
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        rt_kprintf("httpd: create socket failed\n");
        return;
    }
	
	addr.sin_family = AF_INET;
    addr.sin_port = htons(HTTP_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
	
	rt_memset(&(addr.sin_zero), 0, sizeof(addr.sin_zero));
	
	if (bind(server_fd, (struct sockaddr *) &addr, sizeof(struct sockaddr)) == -1)
    {
        PRINTLN("httpd: bind socket failed");
        return;
    }

    if (listen(server_fd, 5) == -1)
    {
        PRINTLN("httpd: listen socket failed");
        return;
    }
	
	while (1)
    {
		if ((client_fd = accept(server_fd, (struct sockaddr *) &addr, &addr_size)) == -1)
        {
            continue;
        }
		
		if ((recv_len = recv(client_fd, recv_buf, RECV_BUF_LEN, 0)) > 0)
        {
            if (recv_buf[0] == ISO_G && recv_buf[1] == ISO_E
					&& recv_buf[2] == ISO_T && recv_buf[3] == ISO_space) {
				/* Find the file we are looking for. */
				for (i = 4; i < 40; ++i) {
					if (recv_buf[i] == ISO_space || recv_buf[i] == ISO_cr
							|| recv_buf[i] == ISO_nl) {
						recv_buf[i] = 0;
						break;
					}
				}
			
				PRINT("request for file "); PRINTLN(&recv_buf[4]);
			
				if (recv_buf[4] == ISO_slash && recv_buf[5] == 0) 
				{
					fs_open(file_index_htm.name, &fsfile);
				} else 
				{
					if (!fs_open((const char *) &recv_buf[4], &fsfile)) 
					{
						PRINTLN("couldn't open file");
						fs_open(file_404_htm.name, &fsfile);
					}
				}
			
				send(client_fd, fsfile.data, fsfile.len, 0);
			}
			else if(recv_buf[0] == ISO_P && recv_buf[1] == ISO_O && recv_buf[2] == ISO_S && recv_buf[3] == ISO_T && (recv_buf[4] == ISO_space || recv_buf[4] == ISO_tab))
			{
				char *start = RT_NULL;
				char *end = RT_NULL;
				char *boundary_value = RT_NULL;
				rt_uint32_t upload_total = 0,upload = 0;
				recv_buf[recv_len] = '\0';
				start = (char *)rt_strstr((char*)recv_buf, "Content-Length:");
				if(start)
				{
					start += 15;
					end = (char *)rt_strstr(start, eol);
					if(end)
					{
						upload_total = rt_atoi(start);
						start = RT_NULL;
						end = RT_NULL;
						start = (char *)rt_strstr((char *)recv_buf, "boundary=");
						if(start)
						{
							// move pointer over "boundary="
							start += 9;
							// find end of line with boundary value
							end = (char *)rt_strstr((char *)start, eol);
							if(end)
							{
								// malloc space for boundary value + '--' and '\0'
								boundary_value = (char*)rt_malloc(end - start + 3);
								if(boundary_value)
								{
									rt_memcpy(&boundary_value[2], start, end - start);

									// add -- at the begin and 0 at the end
									boundary_value[0] = '-';
									boundary_value[1] = '-';
									boundary_value[end - start + 2] = 0;
									
									PRINT("POST：");PRINTLN(boundary_value);
									
									if ((recv_len = recv(client_fd, recv_buf, RECV_BUF_LEN, 0)) > 0)
									{
										start = RT_NULL;
										end = RT_NULL;
										start = (char *)rt_strstr((char *)recv_buf, (char *)boundary_value);
										if(start)
										{
											//end = (char *)rt_strstr((char *)start, "name=\"fw_file\"");
											//if(end)
											{
												PRINTLN("Upgrade type: firmware");
												end = RT_NULL;
												// find start position of the data!
												end = (char *)rt_strstr((char *)start, eol2);
												
												//add file
												
												if(end)
												{
													PRINT("Start Receiving:");
													// move pointer over CR LF CR LF
													end += 4;
													upload_total -=  (int)(end - start) - rt_strlen(boundary_value) - 6;
													upload = (unsigned int)(recv_len - (end - (char *)recv_buf));
													
													//add file
													PRINT("#");
													if(upload < upload_total)
													{
														while((recv_len = recv(client_fd, recv_buf, RECV_BUF_LEN, 0)) > 0)
														{
															//add file
															PRINT("#");
															upload += (unsigned int)recv_len;
															if(upload >= upload_total)
															{
																break;
															}															
														}
													}
												}
												PRINTLN("");
												//add file
											}
											//else
											{
												//PRINTLN("Unknown File");
											}
										}
									}
									
									
									rt_free(boundary_value);
								}
							}
						}
						
					}
				}
			}
        }
		
		closesocket(client_fd);
	}
}

/* telnet server */
void http_server(void)
{
	fs_init();
	rt_thread_init(&httpd_thread,
                   "httpd_thread",
                   &httpd_thread_entry,
                   RT_NULL,
                   &httpd_thread_stack[0],
                   sizeof(httpd_thread_stack),10,4);
	
    rt_thread_startup(&httpd_thread);
}
