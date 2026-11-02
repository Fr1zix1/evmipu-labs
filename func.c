#include "func.h"
#include <stdlib.h>
#include <time.h>

void init_forward(int *arr, size_t n) {
    // Прямой обход: каждый элемент хранит индекс следующего (0->1, 1->2)
    for (size_t i = 0; i < n - 1; i++) {
        arr[i] = (int)(i + 1);
    }
    // Последний элемент ссылается на нулевой
    arr[n - 1] = 0; 
}

void init_reverse(int *arr, size_t n) {
    // Обратный обход: нулевой элемент указывает на конец массива
    arr[0] = (int)(n - 1);
    // Остальные элементы указывают на предыдущий (5->4, 4->3)
    for (size_t i = 1; i < n; i++) {
        arr[i] = (int)(i - 1);
    }
}

void init_random(int *arr, size_t n) {
    int *indices = (int*)malloc(n * sizeof(int));
    if (indices == NULL) {
        return;
    }
    
    // Создаем вспомогательный массив с последовательными индексами
    for (size_t i = 0; i < n; i++) {
        indices[i] = (int)i;
    }
    
    srand((unsigned int)time(NULL));
    // Перемешиваем индексы
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = (size_t)(rand() % (i + 1));
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }
    
    // Формируем кольцевой список на основе перемешанных индексов
    for (size_t i = 0; i < n - 1; i++) {
        arr[indices[i]] = indices[i + 1];
    }
    // Замыкаем последний перемешанный индекс на первый
    arr[indices[n - 1]] = indices[0]; 
    free(indices);
}
