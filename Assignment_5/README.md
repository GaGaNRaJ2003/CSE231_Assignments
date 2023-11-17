## Introduction
This code snippet presents a simple way to run multiple tasks at the same time in C++. It uses a custom multithreading approach using something called POSIX threads (`pthread.h`). This allows you to do several tasks simultaneously, making your programs faster and more efficient.

## Code Overview
The code is neatly organized within the `CustomMultithreading` class, providing special methods for doing tasks in parallel. It includes:
- `ExecuteTask`: This method is like a manager that assigns tasks to different worker threads based on what you want to do.
- `parallel_for`: These functions help you run tasks at the same time for both simple lists of things (1D loops) and grids (2D loops).

## How to Use
To use this code in your own projects, follow these steps:
1. Include the `CustomMultithreading` header file (`custom_multithreading.h`) in your C++ code.
2. Use the `parallel_for` function(s) to do tasks in parallel.

### Example Usage:

#### 1D Loop:
```cpp
// Example task for 1D loop
void exampleTask(int index) {
    // Your task here
}

int main() {
    // Run a task on 1000 items with 4 threads
    parallel_for(0, 1000, exampleTask, 4);
    return 0;
}
```

#### 2D Loop:
```cpp
// Example task for 2D loop
void exampleTask2D(int index1, int index2) {
    // Your 2D task here
}

int main() {
    // Run a task on a grid with 2 threads
    parallel_for(0, 1000, 0, 500, exampleTask2D, 2);
    return 0;
}
```

## Contribution
### Gagan's Contribution

Gagan worked on making sure the code could handle running tasks at the same time:
- Created the structure for handling different tasks in the `CustomMultithreading` class.
- Figured out how to assign tasks to different threads and manage them properly.

### Garv's Contribution

Garv helped improve and add more functionality to the code:
- Made sure the tasks were split evenly among the threads for both 1D and 2D loops.
- Added ways to catch errors and keep everything working smoothly while tasks were running.

## Acknowledgments
This way of running tasks concurrently is inspired by POSIX threads, which is a method used in many programming languages to speed things up. Feel free to modify and build upon this code to suit your own needs. 

## Thank you!!
