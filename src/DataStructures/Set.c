#include "ArrayList.h"
#include "Set.h"
Set* createSet(int capacity) {
    ArrayList_Int* data = createArray_Int();
    Set* set = malloc(sizeof(Set));
    set->elements = data;
    return set;
}

void addElement(Set* set, int element) {
    if (contains_Int(set->elements, element)) {
      return;
    }
    add_Int(set->elements, element);
}

void removeElement(Set* set, int element) {
  int index = indexOf_Int(set->elements, element);
  if (index != -1) {
    removeItem_Int(set->elements, index);
  }
}

int containsElement(Set* set, int element) {  
    return contains_Int(set->elements, element);
}

void freeSet(Set* set) {
    freeList_Int(set->elements);
}

int getSize(Set* set) {
  return size_Int(set->elements);
}

