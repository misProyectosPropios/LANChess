#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <stdlib.h>
#include <stdbool.h>

/**
 * Macro to declare the ArrayList structure and function prototypes.
 * Use this in header files (.h).
 */
#define DECLARE_ARRAYLIST(T, Name) \
typedef struct { \
    T* data; \
    size_t size; \
    size_t capacity; \
} ArrayList_##Name; \
\
ArrayList_##Name* createArray_##Name(); \
void add_##Name(ArrayList_##Name *list, T value); \
T get_##Name(ArrayList_##Name* list, int index); \
bool contains_##Name(ArrayList_##Name* list, T value); \
void freeList_##Name(ArrayList_##Name *list); \
void destroy_##Name(ArrayList_##Name *list); \
bool isEmpty_##Name(ArrayList_##Name *list); \
void removeItem_##Name(ArrayList_##Name *list, int index); \
void set_##Name(ArrayList_##Name *list, int index, T value); \
size_t size_##Name(ArrayList_##Name *list); \
void clear_##Name(ArrayList_##Name *list); \
int indexOf_##Name(ArrayList_##Name* list, T value);

DECLARE_ARRAYLIST(int, Int)

#endif
