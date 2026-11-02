#ifndef FUNC_H
#define FUNC_H

#include <stddef.h>

// Инициализация массива для прямого обхода
void init_forward(int *arr, size_t n);
// Инициализация массива для обратного обхода
void init_reverse(int *arr, size_t n);
// Инициализация массива для случайного обхода
void init_random(int *arr, size_t n);

#endif
