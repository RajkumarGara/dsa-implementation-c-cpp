#ifndef ARRAY_ADT_H
#define ARRAY_ADT_H

struct array {
	int a[10];
	int size;
	int length;
};

void display(struct array arr);
void append(struct array *arr, int element);
void insert(struct array *arr, int index, int element);
int Delete(struct array *arr, int index);
int LinearSearch(struct array arr, int key);

#endif
