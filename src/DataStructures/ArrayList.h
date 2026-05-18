#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    int *data;
    size_t size;
    size_t capacity;
} ArrayList;

ArrayList* createArray();
void add(ArrayList *list, int value);
int get(ArrayList *list, int index);
bool contains(ArrayList *list, int value); 
void freeList(ArrayList *list);
bool isEmpty (ArrayList *list);
void removeItem(ArrayList *list, int index);
void set(ArrayList *list, int index, int value);
size_t size(ArrayList *list);
void clear(ArrayList *list);

int indexOf(ArrayList* list, int value);
#endif
