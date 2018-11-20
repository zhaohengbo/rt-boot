/**
 * \addtogroup httpd
 * @{
 */

/**
 * \file
 * HTTP server read-only file system code.
 * \author Adam Dunkels <adam@dunkels.com>
 *
 * A simple read-only filesystem. 
 */

/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: fs.c,v 1.7.2.3 2003/10/07 13:22:27 adam Exp $
 */

#include <net/uip/uip.h>
#include <net/uip-httpd/httpd.h>
#include <net/uip-httpd/fs.h>
#include <net/uip-httpd/fsdata.h>

#define NULL (void *)0
#include "fsdata.c"

/*-----------------------------------------------------------------------------------*/
static u8_t fs_strcmp(const char *str1, const char *str2) {
	u8_t i;
	i = 0;
	loop:

	if (str2[i] == 0 || str1[i] == '\r' || str1[i] == '\n') {
		return 0;
	}

	if (str1[i] != str2[i]) {
		return 1;
	}

	++i;
	goto loop;
}
/*-----------------------------------------------------------------------------------*/
int fs_open(const char *name, struct fs_file *file) {
	struct fsdata_file *f;

	for (f = (struct fsdata_file *) FS_ROOT; f != NULL ; f =
			(struct fsdata_file *) f->next) {

		if (fs_strcmp(name, f->name) == 0) {
			
			if(f->file_type == FS_STATIC_FILE)
			{
				file->data = f->data;
				file->len = f->len;
				return 1;
			}
			else if(f->file_type == FS_DYNAMIC_FILE)
			{
				u16_t dynamic_len = 0;
				if(f->dynamic_caller)
					dynamic_len = f->dynamic_caller(&(f->data[f->static_len]),f->len-f->static_len);
				file->data = f->data;
				file->len = f->static_len + dynamic_len;
				return 1;
			}
		}
	}
	return 0;
}
/*-----------------------------------------------------------------------------------*/
void fs_init(void) {
	fsdata_init();
}
