#if !defined(MAIN_H)
#define MAIN_H

#include <string.h>

void walk_free_list();
void walk_allocated_chunks();
void audit();

#endif // MAIN_H