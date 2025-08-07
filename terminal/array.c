#include <stdlib.h>
#include <stdbool.h>

#include "array.h"


int initArray(IntArray *arr) {
    arr->data = malloc(ArrayInitialSize * sizeof(int));
    if (arr->data == NULL) return 1;

    arr->capacity = ArrayInitialSize;
    arr->count = 0;
    return 0;
}

int insertArray(IntArray *arr, int value) {
    if (arr->count == arr->capacity) {
        arr->capacity *= 2;
        int *temp = realloc(arr->data, arr->capacity * sizeof(int));
        if (temp == NULL) return 1;
        arr->data = temp;
    }

    arr->data[arr->count++] = value;
    return 0;
}

bool containsArray(const IntArray *arr, int value) {
    for (int i = 0; i < arr->count; i++) {
        if (arr->data[i] == value) {
            return true;
        }
    }
    return false;
}

void freeArray(IntArray *arr) {
    free(arr->data);
    arr->data = NULL;
    arr->count = 0;
    arr->capacity = 0;
}
