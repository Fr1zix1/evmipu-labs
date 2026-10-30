#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <xmmintrin.h> // Библиотека с инструкциями SSE

// Выделение памяти с выравниванием по 16 байт
static float *allocateMemory(int size) {
    return (float *)aligned_alloc(16, (size_t)size * size * sizeof(float));
}

// Заполнение стартовой матрицы с диагональным преобладанием
static void fillStartMatrix(float *matrixA, int size) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            // Если мы на диагонали, ставим большое число
            if (i == j) {
                matrixA[i * size + j] = (float)size + 1.0f;
            } else {
                // Остальные ячейки заполняем единицами
                matrixA[i * size + j] = 1.0f;
            }
        }
    }
}

// Векторизованное умножение матриц по 4 элемента за один шаг
static void multiplyMatricesSse(const float *matrixA, const float *matrixB, float *matrixResult, int size) {
    // Обнуляем матрицу результата
    memset(matrixResult, 0, (size_t)size * size * sizeof(float));
    for (int i = 0; i < size; ++i) {
        for (int k = 0; k < size; ++k) {
            // Копируем один элемент из А во все 4 ячейки векторного регистра
            __m128 elementVectorA = _mm_set1_ps(matrixA[i * size + k]);
            int j = 0;
            // Обрабатываем столбцы блоками по 4 штуки
            for (; j <= size - 4; j += 4) {
                // Загружаем в процессор 4 элемента результата и 4 элемента матрицы B
                __m128 vectorC = _mm_loadu_ps(&matrixResult[i * size + j]);
                __m128 vectorB = _mm_loadu_ps(&matrixB[k * size + j]);
                // Векторно умножаем и складываем их за одну инструкцию
                vectorC = _mm_add_ps(vectorC, _mm_mul_ps(elementVectorA, vectorB));
                // Сохраняем 4 новых числа обратно в матрицу
                _mm_storeu_ps(&matrixResult[i * size + j], vectorC);
            }
            // Если размер не кратен 4, добиваем остаток обычным умножением
            for (; j < size; ++j) {
                matrixResult[i * size + j] += matrixA[i * size + k] * matrixB[k * size + j];
            }
        }
    }
}

// Векторизованное сложение матриц по 4 элемента за такт
static void addMatricesSse(float *matrixSum, const float *matrixAdd, int size) {
    int totalElements = size * size;
    int i = 0;
    // Идем по массиву большими шагами по 4 ячейки
    for (; i <= totalElements - 4; i += 4) {
        // Загружаем по 4 числа из каждой матрицы
        __m128 vectorSum = _mm_loadu_ps(&matrixSum[i]);
        __m128 vectorAdd = _mm_loadu_ps(&matrixAdd[i]);
        // Складываем векторно и сразу возвращаем в память суммы
        _mm_storeu_ps(&matrixSum[i], _mm_add_ps(vectorSum, vectorAdd));
    }
    // Обычное сложение остатка
    for (; i < totalElements; ++i) {
        matrixSum[i] += matrixAdd[i];
    }
}

// Основной алгоритм обращения матрицы
static void calculateInverseSse(const float *matrixA, float *inverseMatrix, int size, int iterationsCount) {
    // Подготовка промежуточных буферов
    float *matrixB = allocateMemory(size);
    float *productBA = allocateMemory(size);
    float *matrixR = allocateMemory(size);
    float *currentPowerR = allocateMemory(size);
    float *nextPowerR = allocateMemory(size);
    float *matrixSum = allocateMemory(size);

    // 1. Поиск максимумов по столбцам и строкам для норм
    float normOne = 0.0f;
    float normInfinity = 0.0f;
    for (int i = 0; i < size; ++i) {
        float rowSum = 0.0f;
        float colSum = 0.0f;
        for (int j = 0; j < size; ++j) {
            rowSum += fabsf(matrixA[i * size + j]);
            colSum += fabsf(matrixA[j * size + i]);
        }
        if (rowSum > normInfinity) {
            normInfinity = rowSum;
        }
        if (colSum > normOne) {
            normOne = colSum;
        }
    }

    // 2. Создание матрицы B с транспонированием
    float scalingFactor = 1.0f / (normOne * normInfinity);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            matrixB[i * size + j] = matrixA[j * size + i] * scalingFactor;
        }
    }

    // 3. Расчет матрицы R с помощью векторного умножения
    multiplyMatricesSse(matrixB, matrixA, productBA, size);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (i == j) {
                // На диагонали
                matrixR[i * size + j] = 1.0f - productBA[i * size + j];
            } else {
                // Вне диагонали
                matrixR[i * size + j] = 0.0f - productBA[i * size + j];
            }
        }
    }

    // 4. Векторное вычисление суммы ряда
    memset(matrixSum, 0, (size_t)size * size * sizeof(float));
    for (int i = 0; i < size; ++i) {
        // Стартовая единичная матрица
        matrixSum[i * size + i] = 1.0f;
    }
    memcpy(currentPowerR, matrixR, (size_t)size * size * sizeof(float));

    for (int iter = 1; iter < iterationsCount; ++iter) {
        // Векторно прибавляем текущую степень
        addMatricesSse(matrixSum, currentPowerR, size);
        if (iter < iterationsCount - 1) {
            // Векторно умножаем для получения следующей степени
            multiplyMatricesSse(currentPowerR, matrixR, nextPowerR, size);
            float *tempPointer = currentPowerR;
            currentPowerR = nextPowerR;
            nextPowerR = tempPointer;
        }
    }

    // 5. Финальное векторное перемножение для ответа
    multiplyMatricesSse(matrixSum, matrixB, inverseMatrix, size);

    // Очистка буферов
    free(matrixB);
    free(productBA);
    free(matrixR);
    free(currentPowerR);
    free(nextPowerR);
    free(matrixSum);
}

// Точка входа в программу
int main(int argc, char **argv) {
    // Чтение параметров
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

    // Работа с таймером
    struct timespec startTime, endTime;
    clock_gettime(CLOCK_MONOTONIC, &startTime);

    // Запуск векторного расчета
    calculateInverseSse(matrixA, inverseMatrix, matrixSize, iterationsCount);

    clock_gettime(CLOCK_MONOTONIC, &endTime);
    double elapsedSeconds = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_nsec - startTime.tv_nsec) * 1e-9;

    // Печать результатов
    printf("[SSE Intrinsics] N = %d, M = %d | Time: %.4f sec\n", matrixSize, iterationsCount, elapsedSeconds);

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
