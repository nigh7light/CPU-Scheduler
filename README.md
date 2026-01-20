# CPU Scheduler

Simple C++ CPU scheduling simulator demonstrating FCFS, SJF, Round Robin and Priority Scheduling.

The `PriorityScheduling` implementation includes an aging mechanism to prevent starvation: every 5 time units a waiting process's numeric priority is decreased by 1 (improving priority). Change the aging interval by editing the `agingInterval` constant in `PriorityScheduling` inside `cpuScheduler.cpp`.

Files
- `cpuScheduler.cpp` : main source implementing algorithms and a small example process set.
- `README.md` : this file.

Build
```bash
g++ -std=c++11 cpuScheduler.cpp -o cpuScheduler
```

Run
```bash
./cpuScheduler
```

Usage
- Choose an algorithm from the menu (1–5).
- For Round Robin, you'll be prompted for a time quantum.
- For Priority Scheduling, the program now applies aging to waiting processes (default interval = 5 time units).

Customize
- Edit the example processes in `main()` within `cpuScheduler.cpp` (vector of `Process(...)`).
- Adjust aging behavior by changing `const int agingInterval = 5;` in the `PriorityScheduling` function.

Notes
- Lower numeric priority means higher scheduling priority.
- The program prints per-process stats and a simple Gantt chart.

If you want, I can run a sample Priority Scheduling execution and show the output.