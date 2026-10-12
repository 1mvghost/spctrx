#include <assert.h>
#include <ll.h>

void llInitHead(LLHead* head) {
  ASSERT(head != 0);

  head->prev = head;
  head->next = head;
}

void llInsertFront(LLHead* head, LLHead* node) {
  ASSERT(head != 0);
  ASSERT(node != 0);

  LLHead* next = head->next;

  node->next = next;
  next->prev = node;
  head->next = node;
  node->prev = head;
}

void llDelete(LLHead* node) {
  ASSERT(node != 0);

  node->prev->next = node->next;
  node->next->prev = node->prev;
}

bool llEmpty(LLHead* head) {
  ASSERT(head != 0);

  return head->next == head;
}