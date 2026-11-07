#ifndef FUNC_H
#define FUNC_H

#include <stddef.h>
#include <stdint.h>

void init_array(uint32_t *arr, size_t n_fragments, size_t frag_size_bytes, size_t offset_bytes);
double measure_time(uint32_t *arr, size_t total_elements, size_t iterations);

#endif
