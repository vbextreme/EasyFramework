#ifdef _APP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "easylist.h"
#include <easyalloc.h>

typedef struct _NODE
{
	struct _NODE* next;
	struct _NODE* prev;
	INT32 i;
}NODE;

typedef struct _LST
{
	NODE* head;
	NODE* tail;
}LST;


int main()
{
	LST l;
	
	lst_init(&l);
	
	NODE* n = alc_new(NODE,1);
	ele_init(n);
	n->i = 0;
	lst_add(&l,n,LLAST,n);
	
	n = alc_new(NODE,1);
	ele_init(n);
	n->i = 1;
	lst_add(&l,n,LLAST,n);
	
	n = alc_new(NODE,1);
	ele_init(n);
	n->i = 2;
	lst_add(&l,n,LLAST,n);
	
	n = alc_new(NODE,1);
	ele_init(n);
	n->i = 3;
	lst_add(&l,n,LLAST,n);
	
	printf("(head)"); lst_debug_print(l.head); printf("  (tail)"); lst_debug_print(l.tail);	putchar('\n');
	forlst(&l,n)
	{
		printf("(%d)",n->i);
		lst_debug_print(n);
		putchar('\n');
	}
	
	putchar('\n');
	
	forlstfree(&l,n)
	{
		free(n);
	}
	
	
	
	return 0;
}

#endif
