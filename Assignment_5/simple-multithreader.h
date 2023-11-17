#ifndef CUSTOM_MULTITHREADING_H
#define CUSTOM_MULTITHREADING_H

#include <iostream>
#include <functional>
#include <pthread.h>
#include <chrono>

class CustomMultithreading {
public:
    // Task execution details structure
    struct task_detail { 
             std::function<void(int)> singleTask;      // Task for 1D loop
        std::function<void(int, int)> doubleTask; // Task for 2D loop
        int startIdx;           //   task execution Start index
        int endIdx;             // task execution End index  
        int lowLimit2;          // Lower limit for the second dimension 
        int highLimit2;         // Upper limit for the second dimension 

    };

    // Function used by pthread_create
    static void* ExecuteTask(void* arg) {
        task_detail* task_inf = static_cast<task_detail*>(arg);
        if (task_inf->singleTask) {
            for (int i = task_inf->startIdx; i < task_inf->endIdx; ++i) {
                task_inf->singleTask(i);
            }
        } else if (task_inf->doubleTask) {
            for (int i = task_inf->startIdx; i < task_inf->endIdx; ++i) {
                for (int j = task_inf->lowLimit2; j < task_inf->highLimit2; ++j) {
                    task_inf->doubleTask(i, j);
                }
            }
        }
        delete task_inf;
        return nullptr;
    }

    // Custom parallel_for for 1D loop
    static void parallel_for(int low, int high, std::function<void(int)> task, int numThreads) {
        auto time_starting = 
        std::chrono::high_resolution_clock::now();

        pthread_t threads[numThreads];
        int dif=high - low;
        int segmentSize = dif / numThreads;

        for (int j = 1; j <= numThreads; ++j) {
            int i=j-1;
            task_detail* task_inf = new task_detail{ 
              task,
                nullptr,
                i * segmentSize,
                (i == numThreads - 1) ? high : (i + 1) * segmentSize,
                0, 0,
               
            };
            pthread_create(&threads[i], nullptr, ExecuteTask, static_cast<void*>(task_inf));
        }

        for (int j = 1; j <= numThreads; ++j) {
          int i=j-1;
            pthread_join(threads[i], nullptr);
        }

        auto time_ending = std::chrono::high_resolution_clock::now();
        // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(time_ending - time_starting).count();
        auto duration = std::chrono::duration<double>(time_ending - time_starting).count();
        
        std::cout << "Time spent in parallel_for: " <<std::fixed << duration << " Seconds" << std::endl;

    }

    // Custom parallel_for for 2D loop
    static void parallel_for(int low1, int high1, int low2, int high2, std::function<void(int, int)> task, int numThreads) {
         auto time_starting = std::chrono::high_resolution_clock::now();

        pthread_t threads[numThreads];
        int segmentSize1 = (high1 - low1) / numThreads;

        for (int j = 1; j <= numThreads; ++j) {
int i=j-1;
            task_detail* task_inf = new task_detail{
              nullptr,
                task,
                i * segmentSize1,
                (i == numThreads - 1) ? high1 : (i + 1) * segmentSize1,
                low2,
                high2,
                
            };
            pthread_create(&threads[i], nullptr, ExecuteTask, static_cast<void*>(task_inf));
        }

        for (int j = 1; j <= numThreads; ++j) {
          int i=j-1;
            pthread_join(threads[i], nullptr);
        }

        auto time_ending = std::chrono::high_resolution_clock::now();
        // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(time_ending - time_starting).count();
        auto duration = std::chrono::duration<double>(time_ending - time_starting).count();
        
        std::cout << "Time spent in parallel_for: " <<std::fixed << duration << " Seconds" << std::endl;
    }
};

// Redefine parallel_for for one-dimensional loop
template <typename F>
static void parallel_for(int low, int high, F task, int numThreads) {
    CustomMultithreading::parallel_for(low, high, task, numThreads);
}

// Redefine parallel_for for two-dimensional loop
template <typename F>
static void parallel_for(int low1, int high1, int low2, int high2, F task, int numThreads) {
    CustomMultithreading::parallel_for(low1, high1, low2, high2, task, numThreads);
}

#endif  
// CUSTOM_MULTITHREADING_H
