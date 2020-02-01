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

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static unsigned int alloc_count = 0;
static unsigned int sweep_count = 0;

void handle_alloc(int argc, char **argv);
void handle_sweep(int argc, char **argv);

typedef struct _handler_t
{
	char* name;
	void (*callback)(int argc, char **argv);
} handler_t;

handler_t handlers[] = {
	{ "alloc", handle_alloc },
	{ "sweep", handle_sweep },
	{ 0, 0 }
};


int main(int argc, char **argv)
{
	char line[1024];
	char* s[8];
	int num, i;
	
	while (!feof(stdin)) {
		fgets(line, 1024, stdin);
		
		num = 0;
		s[num++] = line;

		for (i = 0; line[i] != 0; i++) {
			if (line[i] == '\t') {
				line[i] = 0;
				s[num++] = line + 1;
			}
		}

		s[num] = 0;
		
		if (num > 0) {
			for (i = 0; handlers[i].name != 0; i++) {
				if (strcmp(handlers[i].name, s[0]) == 0)
					(*handlers[i].callback)(num, s);
			}
		}
	}

	printf("alloc_count\t%d\n", alloc_count);
	printf("sweep_count\t%d\n", sweep_count);

	return 0;
}

void handle_alloc(int argc, char **argv)
{
	alloc_count++;
}

void handle_sweep(int argc, char **argv)
{
	sweep_count++;
}
