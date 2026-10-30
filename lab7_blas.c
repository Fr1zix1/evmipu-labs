#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <cblas.h> // Интерфейс к функциям высокопроизводительной библиотеки BLAS

// Выделение памяти с выравниванием по 32 байта
static float *allocateMemory(int size) {
    return (float *)aligned_alloc(32, (size_t)size * size * sizeof(float));
}

// Заполнение стартовой матрицы
static void fillStartMatrix(float *matrixA, int size) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (i == j) {
                matrixA[i * size + j] = (float)size + 1.0f;
            } else {
                matrixA[i * size + j] = 1.0f;
            }
        }
    }
}

// Алгоритм обращения через готовые функции BLAS
static void calculateInverseBlas(const float *matrixA, float *inverseMatrix, int size, int iterationsCount) {
    // Подготавливаем промежуточную память
    float *matrixB = allocateMemory(size);
    float *matrixR = allocateMemory(size);
    float *currentPowerR = allocateMemory(size);
    float *nextPowerR = allocateMemory(size);
    float *matrixSum = allocateMemory(size);

    // 1. Вычисляем матричные нормы
    float normOne = 0.0f;
    float normInfinity = 0.0f;
    for (int i = 0; i < size; ++i) {
        float rowSum = 0.0f;
        float colSum = 0.0f;
        for (int j = 0; j < size; ++j) {
            rowSum += fabsf(matrixA[i * size + j]);
            colSum += fabsf(matrixA[j * size + i]);
        }
        // Обновляем максимумы
        if (rowSum > normInfinity) {
            normInfinity = rowSum;
        }
        if (colSum > normOne) {
            normOne = colSum;
        }
    }

    // 2. Формируем матрицу B
    float scalingFactor = 1.0f / (normOne * normInfinity);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            matrixB[i * size + j] = matrixA[j * size + i] * scalingFactor;
        }
    }

    // Подготавливаем единичную матрицу
    memset(matrixR, 0, (size_t)size * size * sizeof(float));
    for (int i = 0; i < size; ++i) {
        matrixR[i * size + i] = 1.0f;
    }

    // 3. Вычисляем R = I - B * A с помощью библиотечного перемножения
    // Функция cblas_sgemm делает формулу C = alpha * A * B + beta * C
    // Мы передаем alpha = -1, beta = 1, чтобы получить: R = -1 * B * A + 1 * R
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 
                size, size, size, -1.0f, matrixB, size, matrixA, size, 1.0f, matrixR, size);

    // Подготавливаем единичную матрицу для старта суммы
    memset(matrixSum, 0, (size_t)size * size * sizeof(float));
    for (int i = 0; i < size; ++i) {
        matrixSum[i * size + i] = 1.0f;
    }
    memcpy(currentPowerR, matrixR, (size_t)size * size * sizeof(float));

    // 4. Считаем сумму ряда, вызывая BLAS на каждом шаге
    for (int iter = 1; iter < iterationsCount; ++iter) {
        // Функция cblas_saxpy складывает векторы: Y = alpha * X + Y
        // Используем это, чтобы прибавить текущую степень к общей сумме
        cblas_saxpy(size * size, 1.0f, currentPowerR, 1, matrixSum, 1); 
        
        if (iter < iterationsCount - 1) {
            // Умножаем матрицы через BLAS (alpha=1.0, beta=0.0) для получения новой степени
            cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 
                        size, size, size, 1.0f, currentPowerR, size, matrixR, size, 0.0f, nextPowerR, size);
            // Переставляем указатели, чтобы не тратить время на копирование памяти
            float *tempPointer = currentPowerR;
            currentPowerR = nextPowerR;
            nextPowerR = tempPointer;
        }
    }

    // 5. Финальное умножение S * B для получения обратной матрицы
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 
                size, size, size, 1.0f, matrixSum, size, matrixB, size, 0.0f, inverseMatrix, size);

    // Очищаем память
    free(matrixB);
    free(matrixR);
    free(currentPowerR);
    free(nextPowerR);
    free(matrixSum);
}

// Точка входа в программу
int main(int argc, char **argv) {
    // Получение размеров
    int matrixSize = 2048;
    if (argc > 1) {
        matrixSize = atoi(argv[1]);
    }
    
    int iterationsCount = 10;
    if (argc > 2) {
        iterationsCount = atoi(argv[2]);
    }

    // Выделение памяти
    float *matrixA = allocateMemory(matrixSize);
    float *inverseMatrix = allocateMemory(matrixSize);
    fillStartMatrix(matrixA, matrixSize);

    // Запуск таймера
    struct timespec startTime, endTime;
    clock_gettime(CLOCK_MONOTONIC, &startTime);

    // Вызов вычислений с BLAS
    calculateInverseBlas(matrixA, inverseMatrix, matrixSize, iterationsCount);

    // Остановка таймера
    clock_gettime(CLOCK_MONOTONIC, &endTime);
    double elapsedSeconds = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_nsec - startTime.tv_nsec) * 1e-9;

    // Печать результатов
    printf("[BLAS] N = %d, M = %d | Time: %.4f sec\n", matrixSize, iterationsCount, elapsedSeconds);

    // Блок проверки корректности (сработает только для маленьких матриц, например N=4)
    if (matrixSize <= 4) {
        printf("\nПроверка результата (A * A_inv):\n");
        for (int i = 0; i < matrixSize; ++i) {
            for (int j = 0; j < matrixSize; ++j) {
                float checkSum = 0.0f;
                // Умножаем строку исходной матрицы на столбец обратной
                for (int k = 0; k < matrixSize; ++k) {
                    checkSum += matrixA[i * matrixSize + k] * inverseMatrix[k * matrixSize + j];
                }
                // Выводим результат на экран
                printf("%6.3f ", checkSum);
            }
            printf("\n");
        }
        printf("(Должна получиться единичная матрица: 1.0 по диагонали, остальное близко к 0.0)\n");
    }

    free(matrixA);
    free(inverseMatrix);
    return 0;
}
