/*  
    sgc: A garbage collector for C/C++.
 
    Copyright (C) 2002-2008 Peter Hanappe, Sony Computer Science
    Laboratory Paris

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/*  -*- linux-c -*- */

#include "sgc.h"
#include "sgc_log.h"

#ifdef USE_LOGGING

#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>

#include "sgc_list.h"
 
static int sgc_log_initialized = 0;
static sgc_list_t *sgc_log_listeners[SGC_LOG_LAST][8];
static int sgc_stream = -1;

int sgc_log_init()
{
	int i, k;

	if (sgc_log_initialized == 0) {
		sgc_log_initialized = 1;

		for (i = 0; i < SGC_LOG_LAST; i++) {
			for (k = 0; k < 8; k++)
				sgc_log_listeners[i][k] = NULL;
		}
	}
	return 0;
}

void sgc_log_cleanup()
{
	int i, k;

	for (i = 0; i < SGC_LOG_LAST; i++) {
		for (k = 0; k < 8; k++) {
			if (sgc_log_listeners[i][k] != NULL) {
				sgc_list_t* l = sgc_log_listeners[i][k];
				sgc_log_listeners[i][k] = NULL;
				delete_sgc_list(l);
			}
		}
	}
}

void sgc_log(char *file, int line, int type, int filter, char *fmt, ...)
{
	va_list args; 
	char buf[1024];
	sgc_list_t *listeners = sgc_log_listeners[type][filter];
	
	if (listeners == NULL)
		return;

	va_start(args, fmt); 
	vsnprintf(buf, 1024, fmt, args); 
	va_end (args); 

	while (listeners) {
		sgc_log_callback callback = (sgc_log_callback) sgc_list_get(listeners);
		(*callback)(type, filter, buf);
		listeners = sgc_list_next(listeners);
	}
}

void sgc_add_log_listener(int type, int filter, sgc_log_callback write)
{	
	if (sgc_log_initialized == 0)
		sgc_log_init();

	sgc_log_listeners[type][filter] = sgc_list_prepend(sgc_log_listeners[type][filter], 
							   write);
}

void sgc_remove_log_listener(int type, int filter, sgc_log_callback write)
{
	sgc_log_listeners[type][filter] = sgc_list_remove(sgc_log_listeners[type][filter], 
							  write);
}


void sgc_trace(int type, int filter, char *s)
{
	if (sgc_stream != -1) {
		char buf[1024];
		snprintf(buf, 1023, "%s\n", s);
                buf[1023] = 0;

                int len = strlen(buf);
                int n = 0;
		while (n < len) {
                        int r = write(sgc_stream, buf + n, len - n);
                        if (r < 0)
                                // FIXME
                                break;
                        n += r;
                }
        }
}

int sgc_create_stream(int port)
{
	int sock;
	struct sockaddr_in addr;
	
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		printf("Cannot create server socket\n");
		return -1;
	}
	
	memset((char *)&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(sock, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == -1) {
		printf("Cannot bind server socket\n");
		close(sock);
		return -1;
	}

	if (listen(sock, 10) == -1) {
		printf("Cannot listen on server socket\n");
		close(sock);
		return -1;
	}

	printf("Waiting connection\n");

	sgc_stream = accept(sock, NULL, NULL);
	if (sgc_stream == -1) {
		printf("Cannot accept() connection\n");
		return -1;
	}

	close(sock);

        sgc_add_log_listener(SGC_LOG_TRACE, SGC_TRACE_ALLOC, sgc_trace);
        sgc_add_log_listener(SGC_LOG_TRACE, SGC_TRACE_SCAN, sgc_trace);
        sgc_add_log_listener(SGC_LOG_TRACE, SGC_TRACE_GC, sgc_trace);
        
	return 0;
}

void sgc_close_stream(void)
{
	if (sgc_stream != -1)
		close(sgc_stream);
}


#else

int sgc_log_init()
{
        return 0;
}

void sgc_log_cleanup() 
{
}

void sgc_log(char *file, int line, int type, int filter, char *fmt, ...)
{
}

void sgc_add_log_listener(int type, int filter, sgc_log_callback write)
{	
}

void sgc_remove_log_listener(int type, int filter, sgc_log_callback write)
{
}

int sgc_create_stream(int port)
{
	return 0;
}

void sgc_close_stream(void)
{
}

#endif

