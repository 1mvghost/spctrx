#include <assert.h>
#include <ll.h>

void llInitHead(LLHead* head) {
  ASSERT(head != 0);

  head->Prev = head;
  head->Next = head;
}

void llInsertFront(LLHead* head, LLHead* node) {
  ASSERT(head != 0);
  ASSERT(node != 0);

  LLHead* next = head->Next;

  node->Next = next;
  next->Prev = node;
  head->Next = node;
  node->Prev = head;
}

void llDelete(LLHead* node) {
  ASSERT(node != 0);

  node->Prev->Next = node->Next;
  node->Next->Prev = node->Prev;
}

bool llEmpty(LLHead* head) {
  ASSERT(head != 0);

  return head->Next == head;
}