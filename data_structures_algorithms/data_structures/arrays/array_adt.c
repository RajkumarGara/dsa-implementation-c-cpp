#include <stdio.h>
#include "array_adt.h"
#include "../../utils/utils.h"

void display(struct array arr) 
{
	printf("Array:\n");
	for(int i=0; i<arr.length; i++)
		printf("%d\n", arr.a[i]);
}

void append(struct array *arr, int element)
{
	if(arr->length < arr->size)
	{
		arr->a[arr->length++] = element;
	}
}

void insert(struct array *arr, int index, int element)
{
	if(index>=0 && index<=arr->length && arr->length < arr->size)
	{
		for(int i=arr->length; i>index; i--)
		{
			arr->a[i] = arr->a[i-1];	
		}
		arr->a[index] = element;
		arr->length++;
	}	
}

int Delete(struct array *arr, int index)
{
	int element=0;
	if(index>=0 && index<arr->length)
	{
		element = arr->a[index];
		for(int i=index; i<arr->length-1; i++)
		{
			arr->a[i] = arr->a[i+1];
		}
		arr->length--;
		return element;
	}
	return 0;
}

int LinearSearch(struct array arr, int key)
{
	for(int i=0; i<arr.length; i++)
	{
		if(arr.a[i] == key)
		{
			if(i>0)	//key is already in index 0; no need to perform transposition
				swap(&arr.a[i], &arr.a[i-1]);
			return i;
		}
	}
	return -1;
}

int main(void)
{
	struct array arr = {{3,7,2,8,9}, 10, 5};
	
	display(arr);

	append(&arr, 6);
	display(arr);

	insert(&arr, 3, 23);
	display(arr);

	printf("deleted element: %d\n",Delete(&arr, 5));
	display(arr);

	printf("Linear Search index: %d\n",LinearSearch(arr, 8));

	return 0;
}
