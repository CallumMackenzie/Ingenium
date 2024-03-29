#include "Memory.h"

#include "LinkedList.h"

DynamicLinkedList::~DynamicLinkedList()
{
	Node* ptr = head;
	while (ptr != nullptr) {
		Node* tmpPtr = ptr->next;
		memory::safe_delete(ptr);
		ptr = tmpPtr;
	}
}

void DynamicLinkedList::add(void* data, int type)
{
	Node* ptr = head;
	if (head == nullptr) {
		head = new Node();
		head->data = data;
		head->type = type;
	}
	else {
		while (ptr->next != nullptr) {
			ptr = ptr->next;
		}
		ptr->next = new Node();
		ptr->next->data = data;
		ptr->next->type = type;
	}
}

int DynamicLinkedList::length()
{
	int total = 0;
	Node* ptr = head;
	while (ptr != nullptr) {
		total++;
		ptr = ptr->next;
	}
	return total;
}

DynamicLinkedList::Node* DynamicLinkedList::get(int index)
{
	Node* ptr = head;
	for (int i = 0; i < index; i++) {
		ptr = ptr->next;
	}
	return ptr;
}
