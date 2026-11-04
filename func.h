#ifndef FUNC_H
#define FUNC_H

#include <stddef.h>
#include <stdint.h>

void init_forward(int *arr, size_t n);
void init_backward(int *arr, size_t n);
void init_random(int *arr, size_t n);
uint64_t measure_time(int *arr, size_t n, size_t k);

#endif
