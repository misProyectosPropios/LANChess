#ifndef SET_H
#define SET_H

#include "ArrayList.h"

typedef struct Set {
    ArrayList* elements;
} Set;

Set* createSet(int capacity);
void addElement(Set* set, int element);
void removeElement(Set* set, int element);
int containsElement(Set* set, int element);
void freeSet(Set* set);
int getSize(Set* set);

#endif
