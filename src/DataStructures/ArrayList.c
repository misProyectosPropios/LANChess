#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "board.h"
#include "ArrayList.h"

/**
 * Macro to implement the ArrayList for a specific type.
 */
#define IMPLEMENT_ARRAYLIST(T, Name) \
ArrayList_##Name* createArray_##Name() { \
    ArrayList_##Name* list = (ArrayList_##Name*)malloc(sizeof(ArrayList_##Name)); \
    if (list == NULL) return NULL; \
    list->size = 0; \
    list->capacity = 5; \
    list->data = (T*)malloc(list->capacity * sizeof(T)); \
    if (list->data == NULL) { \
        free(list); \
        return NULL; \
    } \
    return list; \
} \
\
void add_##Name(ArrayList_##Name *list, T value) { \
    if (list == NULL) return; \
    if (list->size >= list->capacity) { \
        size_t new_capacity = list->capacity * 2; \
        T* new_data = (T*)realloc(list->data, new_capacity * sizeof(T)); \
        if (new_data == NULL) return; \
        list->data = new_data; \
        list->capacity = new_capacity; \
    } \
    list->data[list->size++] = value; \
} \
\
T get_##Name(ArrayList_##Name* list, int index) { \
    if (list == NULL || index < 0 || (size_t)index >= list->size) { \
        fprintf(stderr, "Error: Index out of bounds.\n"); \
        static T empty; \
        return empty; \
    } \
    return list->data[index]; \
} \
\
bool contains_##Name(ArrayList_##Name* list, T value) { \
    if (list == NULL) return false; \
    for (size_t i = 0; i < list->size; i++) { \
        if (memcmp(&list->data[i], &value, sizeof(T)) == 0) return true; \
    } \
    return false; \
} \
\
void freeList_##Name(ArrayList_##Name *list) { \
    if (list == NULL) return; \
    free(list->data); \
    list->data = NULL; \
    list->size = list->capacity = 0; \
} \
\
void destroy_##Name(ArrayList_##Name *list) { \
    if (list == NULL) return; \
    free(list->data); \
    free(list); \
} \
\
bool isEmpty_##Name(ArrayList_##Name *list) { \
    return list == NULL || list->size == 0; \
} \
\
void removeItem_##Name(ArrayList_##Name *list, int index) { \
    if (list == NULL || index < 0 || (size_t)index >= list->size) return; \
    for (size_t i = (size_t)index; i < list->size - 1; i++) { \
        list->data[i] = list->data[i + 1]; \
    } \
    list->size--; \
} \
\
void set_##Name(ArrayList_##Name *list, int index, T value) { \
    if (list == NULL || index < 0 || (size_t)index >= list->size) return; \
    list->data[index] = value; \
} \
\
size_t size_##Name(ArrayList_##Name *list) { \
    return list ? list->size : 0; \
} \
\
void clear_##Name(ArrayList_##Name *list) { \
    if (list != NULL) list->size = 0; \
} \
\
int indexOf_##Name(ArrayList_##Name* list, T value) { \
    if (list == NULL) return -1; \
    for(size_t i = 0; i < list->size; i++) { \
        if (memcmp(&list->data[i], &value, sizeof(T)) == 0) return (int)i; \
    } \
    return -1; \
}


IMPLEMENT_ARRAYLIST(int, Int)
IMPLEMENT_ARRAYLIST(BoardPosition, BoardPosition)
IMPLEMENT_ARRAYLIST(Movement, Movement)