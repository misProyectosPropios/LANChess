#include "ArrayList.h"
#include "Set.h"
Set* createSet(int capacity) {
    ArrayList* data = createArray();
    Set* set = malloc(sizeof(Set));
    set->elements = data;
    return set;
}

void addElement(Set* set, int element) {
    if (contains(set->elements, element)) {
      return;
    }
    add(set->elements, element);
}

void removeElement(Set* set, int element) {
  int index = indexOf(set->elements, element);
  if (index != -1) {
    removeItem(set->elements, index);
  }
}

int containsElement(Set* set, int element) {  
    return contains(set->elements, element);
}

void freeSet(Set* set) {
    freeList(set->elements);
}

int getSize(Set* set) {
  return size(set->elements);
}

