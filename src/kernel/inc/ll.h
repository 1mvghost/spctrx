#ifndef LL_H
#define LL_H

#include <util.h>

/* taken from
 * https://elixir.bootlin.com/linux/2.1.45/source/include/linux/list.h */
#define LIST_ENTRY(ptr, type, member) \
  ((type*)((char*)(ptr) - (unsigned long)(&((type*)0)->member)))

#define LL_TRAVERSE(curr, head) \
  for (curr = (head)->Next; curr != head; curr = curr->Next)
;
typedef struct _LLHead {
  struct _LLHead *Prev, *Next;
} LLHead;

void llInitHead(LLHead* head);
void llInsertFront(LLHead* head, LLHead* node);
void llDelete(LLHead* node);
bool llEmpty(LLHead* head);

#endif