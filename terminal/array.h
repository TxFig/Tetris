#pragma once

#include <stdbool.h>


typedef struct {
    int *data;
    int count;
    int capacity;
} IntArray;

#define ArrayInitialSize 10

int initArray(IntArray *arr);
int insertArray(IntArray *arr, int value);
bool containsArray(const IntArray *arr, int value);
void freeArray(IntArray *arr);
