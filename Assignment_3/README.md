# Shell Process Scheduler

The Process Scheduler is a component of Shell program that manages task scheduling and execution across multiple CPUs. It implements a round-robin scheduling algorithm to distribute tasks efficiently among available CPU resources.

## Features

- **Round-Robin Scheduling:** Utilizes a round-robin scheduling algorithm to schedule tasks among multiple CPUs.
- **Task Management:** Manages a pool of tasks, assigns priorities, and allocates tasks to available CPUs.
- **Multi-Threading:** Utilizes threads to execute tasks concurrently and manage CPU resources effectively.
- **Task Queue:** Implements a priority-based task queue for efficient task handling.

## Initialization

The process scheduler is initialized within the Shell program by specifying the number of CPU resources (`NCPU`) and the time quantum for each process (`TSLICE`) via command-line arguments.

### Usage

To utilize the process scheduler, follow these steps:

1. **Compilation:** Compile the Aggaris program, including the process scheduler component.

2. **Run Aggaris:** Execute Aggaris with the specified number of CPUs and time slice for process scheduling:

   ```bash
   ./shell <NCPU> <TSLICE>
   ```

3. **Execution:** Aggaris will use the process scheduler to manage task execution based on the specified `NCPU` and `TSLICE`.

## Functionality

### Round-Robin Scheduling

The round-robin scheduling algorithm distributes tasks among available CPU resources in a circular manner. It allocates a fixed time slice (`TSLICE`) to each task for execution, ensuring fair utilization of CPU resources.

### Task Management

- **Task Pool:** Maintains a pool of tasks that need to be executed.
- **Task Priorities:** Assigns priorities to tasks based on certain criteria for efficient execution.
- **Task Execution:** Utilizes threads to execute tasks concurrently on available CPUs.

### Task Queue

The process scheduler manages a priority queue to handle incoming tasks. It retrieves tasks based on priority and allocates CPU resources accordingly.

## Customization

The process scheduler can be customized or extended to include additional scheduling algorithms or task management strategies to suit specific requirements.

## Future Improvements

- **Enhanced Scheduling Algorithms:** Implement different scheduling algorithms for better resource allocation.
- **Task Prioritization:** Enhance task prioritization mechanisms for optimized task execution.
- **Dynamic CPU Allocation:** Explore methods to dynamically allocate CPUs based on task load.


## Contributions

### Gagan's Contributions

- **Process Scheduler Design:** Collaborated in designing the architecture and functionalities of the process scheduler component.
- **Round-Robin Scheduling:** Implemented the round-robin scheduling algorithm for task distribution among CPUs.
- **Task Management:** Contributed to the task pool management, including task prioritization and execution logic.
- **Multi-Threading Implementation:** Developed threading logic for concurrent task execution on available CPUs.
- **Testing and Debugging:** Participated in testing the scheduler, identifying issues, and debugging the implemented functionalities.

### Garv's Contributions

- **Scheduler Integration:** Integrated the process scheduler component into the Aggaris program for task execution.
- **Initialization Logic:** Implemented initialization procedures for the scheduler based on command-line arguments.
- **Customization Support:** Explored possibilities for customizing the scheduler and extending its functionalities.
- **Error Handling and Refinement:** Assisted in error handling strategies and refining the scheduler's logic for better performance.
- **Documentation:** Contributed to writing the README files, explaining the scheduler's features, usage, and future improvements.

---
Thank you!!
