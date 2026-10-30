#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

// Выделение памяти с выравниванием по 16 байт
static float *allocateMemory(int size) {
    return (float *)aligned_alloc(16, (size_t)size * size * sizeof(float));
}

// Заполнение матрицы стартовыми значениями
static void fillStartMatrix(float *matrixA, int size) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            // Если мы на диагонали, ставим большое число, чтобы ряд сошелся
            if (i == j) {
                matrixA[i * size + j] = (float)size + 1.0f;
            } else {
                // Остальные ячейки заполняем единицами
                matrixA[i * size + j] = 1.0f;
            }
        }
    }
}

// Умножение матриц
static void multiplyMatrices(const float *matrixA, const float *matrixB, float *matrixResult, int size) {
    // Предварительно заполняем матрицу результата нулями
    memset(matrixResult, 0, (size_t)size * size * sizeof(float));
    for (int i = 0; i < size; ++i) {
        for (int k = 0; k < size; ++k) {
            // Запоминаем текущий элемент первой матрицы
            float elementA = matrixA[i * size + k];
            for (int j = 0; j < size; ++j) {
                // Умножаем и накапливаем сумму для ячейки результата
                matrixResult[i * size + j] += elementA * matrixB[k * size + j];
            }
        }
    }
}

// Сложение двух матриц как одномерных массивов
static void addMatrices(float *matrixSum, const float *matrixAdd, int size) {
    for (int i = 0; i < size * size; ++i) {
        matrixSum[i] += matrixAdd[i];
    }
}

// Главный алгоритм обращения матрицы
static void calculateInversePlain(const float *matrixA, float *inverseMatrix, int size, int iterationsCount) {
    // Выделяем память под промежуточные матрицы
    float *matrixB = allocateMemory(size);
    float *productBA = allocateMemory(size);
    float *matrixR = allocateMemory(size);
    float *currentPowerR = allocateMemory(size);
    float *nextPowerR = allocateMemory(size);
    float *matrixSum = allocateMemory(size);

    // 1. Считаем нормы матрицы по столбцам и строкам
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

    // 2. Формируем матрицу B = A^T / (||A||_1 * ||A||_inf)
    float scalingFactor = 1.0f / (normOne * normInfinity);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            matrixB[i * size + j] = matrixA[j * size + i] * scalingFactor;
        }
    }

    // 3. Формируем матрицу R = I - B * A
    multiplyMatrices(matrixB, matrixA, productBA, size);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (i == j) {
                // На диагонали вычитаем из единицы
                matrixR[i * size + j] = 1.0f - productBA[i * size + j];
            } else {
                // Вне диагонали вычитаем из нуля
                matrixR[i * size + j] = 0.0f - productBA[i * size + j];
            }
        }
    }

    // 4. Суммируем итерационный ряд S = I + R + R^2 + ...
    memset(matrixSum, 0, (size_t)size * size * sizeof(float));
    for (int i = 0; i < size; ++i) {
        // Начинаем сумму с единичной матрицы
        matrixSum[i * size + i] = 1.0f;
    }
    // Записываем R как первую степень для начала цикла
    memcpy(currentPowerR, matrixR, (size_t)size * size * sizeof(float));

    for (int iter = 1; iter < iterationsCount; ++iter) {
        // Прибавляем текущую степень к общей сумме
        addMatrices(matrixSum, currentPowerR, size);
        if (iter < iterationsCount - 1) {
            // Вычисляем следующую степень матрицы R
            multiplyMatrices(currentPowerR, matrixR, nextPowerR, size);
            float *tempPointer = currentPowerR;
            currentPowerR = nextPowerR;
            nextPowerR = tempPointer;
        }
    }

    // 5. Получаем обратную матрицу умножением суммы на B
    multiplyMatrices(matrixSum, matrixB, inverseMatrix, size);

    // Очищаем всю промежуточную память
    free(matrixB);
    free(productBA);
    free(matrixR);
    free(currentPowerR);
    free(nextPowerR);
    free(matrixSum);
}

// Точка входа в программу
int main(int argc, char **argv) {
    // Считываем параметры запуска или ставим значения по умолчанию
    int matrixSize = 2048;
    if (argc > 1) {
        matrixSize = atoi(argv[1]);
    }
    
    int iterationsCount = 10;
    if (argc > 2) {
        iterationsCount = atoi(argv[2]);
    }

    // Подготавливаем начальную и конечную матрицы
    float *matrixA = allocateMemory(matrixSize);
    float *inverseMatrix = allocateMemory(matrixSize);
    fillStartMatrix(matrixA, matrixSize);

    // Запускаем таймер
    struct timespec startTime, endTime;
    clock_gettime(CLOCK_MONOTONIC, &startTime);

    // Выполняем расчет
    calculateInversePlain(matrixA, inverseMatrix, matrixSize, iterationsCount);

    // Останавливаем таймер и считаем разницу
    clock_gettime(CLOCK_MONOTONIC, &endTime);
    double elapsedSeconds = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_nsec - startTime.tv_nsec) * 1e-9;

    // Выводим время работы
    printf("[Plain C] N = %d, M = %d | Time: %.4f sec\n", matrixSize, iterationsCount, elapsedSeconds);

    // Убираем за собой
    free(matrixA);
    free(inverseMatrix);
    return 0;
}
