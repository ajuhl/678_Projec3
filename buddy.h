#ifndef BUDDY_H
#define BUDDY_H

#include "list.h"

void buddy_init();
void *buddy_alloc(int size);
void buddy_free(void *addr);
void buddy_dump();

#endif // BUDDY_H
