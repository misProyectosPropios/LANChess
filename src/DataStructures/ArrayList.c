#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "ArrayList.h"

ArrayList* createArray() {
    ArrayList* list = (ArrayList*)malloc(sizeof(ArrayList));
    if (list == NULL) return NULL;

    list->size = 0;   
    list->capacity = 5;
    list->data = (int*)malloc(list->capacity * sizeof(int));
    if (list->data == NULL) {
        free(list);
        return NULL;
    }
    return list;
}

void add(ArrayList *list, int value) {
    if (list == NULL) return;
    if (list->size >= list->capacity) {
        size_t new_capacity = list->capacity * 2;
        int* new_data = (int*)realloc(list->data, new_capacity * sizeof(int));
        if (new_data == NULL) return; // Allocation failed
        list->data = new_data;
        list->capacity = new_capacity;
    }
    list->data[list->size++] = value;
}


int get(ArrayList* list, int index) {
    if (list == NULL) return -1;
    if (index < 0 || index >= list->size) {
        fprintf(stderr, "Error: Index out of bounds.\n");
        return -1;
    }
    return list->data[index];
}

bool contains(ArrayList* list, int value) {
    if (list == NULL) return false;
    for (size_t i = 0; i < list->size; i++) {
        if (list->data[i] == value) {
            return true;
        }
    }
    return false;
}
void freeList(ArrayList *list) {
    if (list == NULL) return;
    free(list->data);
    list->data = NULL;
    list->size = list->capacity = 0;
}

bool isEmpty (ArrayList *list) {
    return list == NULL || list->size == 0;
}

void removeItem(ArrayList *list, int index) {
    if (list == NULL) return;
    if (index < 0 || index >= list->size) {
        fprintf(stderr, "Error: Index out of bounds.\n");
        return;
    }
    for (size_t i = index; i < list->size - 1; i++) {
        list->data[i] = list->data[i + 1];
    }
    list->size--;
}

void set(ArrayList *list, int index, int value) {
    if (list == NULL) return;
    if (index < 0 || index >= list->size) {
        fprintf(stderr, "Error: Index out of bounds.\n");
        return;
    }
    list->data[index] = value;
}

size_t size(ArrayList *list) {
    return list ? list->size : 0;
}

void clear(ArrayList *list) {
    if (list != NULL) list->size = 0;
}

int indexOf(ArrayList* list, int value) {
    if (list == NULL) return -1;
    for(size_t i = 0; i < list->size; i++) {
        if (list->data[i] == value) {
            return i;
        }
    }
    return -1;
}
