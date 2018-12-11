#include <kernel/rtthread.h>

#include <net/lwip/lwip/opt.h>
#include <net/lwip/lwip/sockets.h>
#include <net/lwip/lwip/sys.h>
#include <net/lwip/lwip/udp.h>
#include <net/lwip/lwip/debug.h>

#include <dfs/dfs_posix.h>

//#define TFTP_DEBUG

#ifdef TFTP_DEBUG
#define tftp_debug_printf(fmt,args...) rt_kprintf(fmt,##args)
#else
#define tftp_debug_printf(fmt,args...)
#endif

#ifndef TFTP_PORT
#define TFTP_PORT 69
#endif

#ifndef TFTP_TIMEOUT_MSECS
#define TFTP_TIMEOUT_MSECS    10000
#endif

#ifndef TFTP_MAX_RETRIES
#define TFTP_MAX_RETRIES      5
#endif

/*
 *      TFTP operations.
 */
#define RRQ   1
#define WRQ   2
#define DATA  3
#define ACK   4
#define ERROR 5
#define OACK  6

#define TFTP_BLOCK_SIZE     512
#define TFTP_RX_BUF_SIZE    550
#define TFTP_ACK_BUF_SIZE     4


#define FILENAME_SIZE       128

static char remote_file[FILENAME_SIZE];

err_t tftp_start(char *output_file, ip_addr_t remote_ip, char *filename)
{
	int s;
	struct sockaddr_in to;
	struct sockaddr_in from;
	u8_t request[TFTP_BLOCK_SIZE];
	u8_t rec_buf[TFTP_RX_BUF_SIZE];
	u8_t ack_buf[TFTP_ACK_BUF_SIZE];
	int request_len;
	int fromlen, len;
	u8_t *ptr;
	u16_t *op, *u16_ptr, block;
	int err = ERR_OK;
	int cnt = 0, tot_len = 0;
	int finish = 0;
	int fd;
#ifdef TFTP_DEBUG
	char *msg;
	u16_t code;
#endif

	fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0);
	if (fd < 0)
    	return -1;
	rt_strcpy(remote_file, filename);
	remote_file[FILENAME_SIZE-1] = '\0';

	/* create UDP socket */
	if ((s = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		tftp_debug_printf("Can't create socket\n");
		return ERR_MEM;
	}

	/* assign server addr */
	rt_memset(&to, 0, sizeof(struct sockaddr_in));
	to.sin_len = sizeof(to);
	to.sin_family = AF_INET;
	inet_addr_from_ip4addr(&to.sin_addr, &remote_ip);
	to.sin_port = PP_HTONS(TFTP_PORT);

	/* prepare RRQ request */
	rt_memset(request, 0, TFTP_BLOCK_SIZE);
	ptr = request;
	op = (u16_t *) ptr;
	*op = PP_HTONS(RRQ);
	ptr += 2;
	ptr += rt_sprintf((char *)ptr,"%s%c%s%c%s%c%u%c%s%c%d%c", remote_file, 0, "octet", 0, "timeout", 0, TFTP_TIMEOUT_MSECS / 1000, 0, "blksize", 0, TFTP_BLOCK_SIZE, 0);
	request_len = (uint64_t) (ptr - request);

	tftp_debug_printf("TFTP request RRQ len %d\n", request_len);

	if((err = lwip_sendto(s, request, request_len, 0, (struct sockaddr*)&to, sizeof(to))) == -1) {
		tftp_debug_printf("can't send TFTP RRQ, ret = %d\n", err);
		goto OUT;
	}

	rt_memset(rec_buf, 0, TFTP_RX_BUF_SIZE);
	fromlen = sizeof(from);

	do {
		len = lwip_recvfrom(s, rec_buf, sizeof(rec_buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen);
		tftp_debug_printf("recv %d bytes, from port %d\n", len, ntohs(from.sin_port));

		to.sin_port = from.sin_port;
		
		op = (u16_t *) rec_buf;
		switch(ntohs(*op)) {
		case DATA:
			if (len < (TFTP_BLOCK_SIZE + 4))
				finish = 1;
			write(fd, &rec_buf[4], len - 4);
			tot_len += (len - 4);
			u16_ptr = (u16_t *) &rec_buf[2];
			block = ntohs(*u16_ptr);
			ack_buf[2] = rec_buf[2];
			ack_buf[3] = rec_buf[3];
			if ((block % 10) == 0) {
				tftp_debug_printf("#");
				if ((block % 700) == 0)
					tftp_debug_printf("\n");
			}
			cnt = 0;
			break;
		case OACK:
			tftp_debug_printf("recv OACK\n");
			ack_buf[2] = 0;
			ack_buf[3] = 0;
			break;
		case ERROR:
#ifdef TFTP_DEBUG
			tftp_debug_printf("recv ERROR\n");
			u16_ptr = (u16_t *) &rec_buf[2];
			code = ntohs(*u16_ptr);
			msg = (char *) &rec_buf[4];
			tftp_debug_printf("TFTP ERROR: [%s] (%d)\n", msg, code);
#endif
			goto OUT;
		default:
			tftp_debug_printf("TFTP: unsupport op code %d\n", ntohs(*op));
			ack_buf[2] = rec_buf[2];
			ack_buf[3] = rec_buf[3];
			tftp_debug_printf("T");
			cnt++;
		}
		op = (u16_t *) ack_buf;
		*op = PP_HTONS(ACK);
		if((err = lwip_sendto(s, ack_buf, TFTP_ACK_BUF_SIZE, 0, (struct sockaddr*)&to, sizeof(to))) == -1) {
			tftp_debug_printf("can't send TFTP ACK, rx len = %d, ret = %d\n", tot_len, err);
			goto OUT;
		}
	} while ((!finish) && cnt <= TFTP_MAX_RETRIES);
	tftp_debug_printf("\nReceive %s size %d bytes to file %s\n", remote_file, tot_len, output_file);
OUT:
	close(fd);
	lwip_close(s);
	return err;
}
