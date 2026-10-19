#include <iostream>
#include <time.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000.0) + (ts.tv_nsec / 1000000.0);
}

int main() {
    VideoCapture capture(0);
    if (!capture.isOpened()) {
        cerr << "Ошибка: не удалось открыть камеру!" << endl;
        return 0;
    }

    Mat frame, processed;
    
    double total_input = 0.0, total_proc = 0.0, total_show = 0.0, total_wait = 0.0;
    int frames_count = 0;

    double sec_input = 0.0, sec_proc = 0.0, sec_show = 0.0, sec_wait = 0.0;
    int sec_frames = 0;
    double last_report_time = get_time_ms();

    double T1 = get_time_ms(); 

    while (true) {
        double t0 = get_time_ms(); 
        
        capture >> frame; 
        double t1 = get_time_ms();
        
        if (frame.empty()) break;
        
        processed = frame.clone();
        
        blur(processed, processed, Size(25, 25)); 
        for (int y = 0; y < processed.rows; y++) {
            uchar *ptr = processed.data + y * processed.step;
            for (int x = 0; x < processed.cols; x++) {
                ptr[3 * x] = 0;
                ptr[3 * x + 2] = 0;
            }
        }
        double t2 = get_time_ms();

        imshow("Original", frame);
        moveWindow("Original", 100, 100); 
        
        imshow("Effects", processed); 
        moveWindow("Effects", 800, 100); 
        
        double t3 = get_time_ms();

        char c = (char)waitKey(33); 
        
        double t4 = get_time_ms(); 

        double dt_input = t1 - t0;
        double dt_proc  = t2 - t1;
        double dt_show  = t3 - t2;
        double dt_wait  = t4 - t3;
        
        total_input += dt_input;
        total_proc  += dt_proc;
        total_show  += dt_show;
        total_wait  += dt_wait;
        frames_count++;

        sec_input += dt_input;
        sec_proc  += dt_proc;
        sec_show  += dt_show;
        sec_wait  += dt_wait;
        sec_frames++;

        if (t4 - last_report_time >= 1000.0) {
            double sec_cycle_time = t4 - last_report_time;
            double fps = (sec_frames / sec_cycle_time) * 1000.0;
            
            double pct_input = (sec_input / sec_cycle_time) * 100.0;
            double pct_proc  = (sec_proc  / sec_cycle_time) * 100.0;
            double pct_show  = (sec_show  / sec_cycle_time) * 100.0;
            double pct_wait  = (sec_wait  / sec_cycle_time) * 100.0;
            
            printf("FPS: %5.2f | Доли: Ввод %5.1f%%, Обр. %5.1f%%, Вывод %5.1f%%, Ожид. %5.1f%%\n", 
                   fps, pct_input, pct_proc, pct_show, pct_wait);
                   
            sec_input = sec_proc = sec_show = sec_wait = 0.0;
            sec_frames = 0;
            last_report_time = t4;
        }

        if (c == 27) break; 
    }

    double T2 = get_time_ms(); 
    
    double total_time = T2 - T1;

    if (frames_count > 0) {
        double avg_iteration_time = total_time / frames_count;
        
        double pct_total_input = (total_input / total_time) * 100.0;
        double pct_total_proc  = (total_proc  / total_time) * 100.0;
        double pct_total_show  = (total_show  / total_time) * 100.0;
        double pct_total_wait  = (total_wait  / total_time) * 100.0;
        
        printf("\n================ ИТОГОВАЯ СТАТИСТИКА ================\n");
        printf("Всего кадров                   : %d\n", frames_count);
        printf("Общее время работы (T2 - T1)   : %.2f мс\n", total_time);
        printf("Среднее время одной итерации   : %.2f мс\n", avg_iteration_time);
        printf("Средний FPS                    : %.2f\n", (frames_count / total_time) * 1000.0);
        printf("-----------------------------------------------------\n");
        printf("Доли времени от одной итерации:\n");
        printf("  1. Ввод кадра                : %5.2f%% (%.2f мс)\n", pct_total_input, total_input / frames_count);
        printf("  2. Преобразование            : %5.2f%% (%.2f мс)\n", pct_total_proc, total_proc / frames_count);
        printf("  3. Вывод окон                : %5.2f%% (%.2f мс)\n", pct_total_show, total_show / frames_count);
        printf("  4. Ожидание                  : %5.2f%% (%.2f мс)\n", pct_total_wait, total_wait / frames_count);
        printf("=====================================================\n");
    }

    destroyAllWindows();
    return 0;
}
