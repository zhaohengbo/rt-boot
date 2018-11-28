#include <kernel/rtthread.h>
#include <drivers/rtdevice.h>

#include <net/lwip/lwip/sockets.h>

#include <net/lwip-httpd/httpd.h>
#include <net/lwip-httpd/fs.h>
#include <net/lwip-httpd/fsdata.h>

#ifdef HTTPD_DEBUG
#define PRINT(x) rt_kprintf("%s", x)
#define PRINTLN(x) rt_kprintf("%s\n", x)
#else /* HTTPD_DEBUG */
#define PRINT(x)
#define PRINTLN(x)
#endif /* HTTPD_DEBUG */

extern const struct fsdata_file file_index_htm;
extern const struct fsdata_file file_404_htm;

#define ISO_G        0x47
#define ISO_E        0x45
#define ISO_T        0x54
#define ISO_slash    0x2f    
#define ISO_c        0x63
#define ISO_g        0x67
#define ISO_i        0x69
#define ISO_space    0x20
#define ISO_nl       0x0a
#define ISO_cr       0x0d
#define ISO_a        0x61
#define ISO_t        0x74
#define ISO_hash     0x23
#define ISO_period   0x2e

#define HTTP_PORT 80

#define RECV_BUF_LEN 1600
static rt_uint8_t recv_buf[RECV_BUF_LEN];
static rt_int32_t recv_len = 0;

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
            if (recv_buf[0] != ISO_G || recv_buf[1] != ISO_E
					|| recv_buf[2] != ISO_T || recv_buf[3] != ISO_space) {
				/* If it isn't a GET, we abort the connection. */
				closesocket(client_fd);
			}

			/* Find the file we are looking for. */
			for (i = 4; i < 40; ++i) {
				if (recv_buf[i] == ISO_space || recv_buf[i] == ISO_cr
						|| recv_buf[i] == ISO_nl) {
					recv_buf[i] = 0;
					break;
				}
			}
			
			PRINT("request for file "); PRINTLN(&recv_buf[4]);
			
			if (recv_buf[4] == ISO_slash && recv_buf[5] == 0) {
				fs_open(file_index_htm.name, &fsfile);
			} else {
				if (!fs_open((const char *) &recv_buf[4], &fsfile)) {
					PRINTLN("couldn't open file");
					fs_open(file_404_htm.name, &fsfile);
				}
			}
			
			send(client_fd, fsfile.data, fsfile.len, 0);
			closesocket(client_fd);
        }
		else
		{
			closesocket(client_fd);
		}
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
