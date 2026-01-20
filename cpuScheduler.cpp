#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <climits>
#include <string>
#include <limits>

using namespace std;

// Structure to track execution segments for Gantt chart
struct ExecutionSegment {
    int processID;
    int startTime;
    int endTime;
};

class Process {
    private:
        int PID;
        int arrivalTime;
        int burstTime;
        int priority;
        int remainingTime;

    public:
        int completionTime;
        int turnaroundTime;
        int waitingTime;

        Process(int PID, int arrivalTime, int burstTime, int priority) {
            this->PID = PID;
            this->arrivalTime = arrivalTime;
            this->burstTime = burstTime;
            this->priority = priority;
            this->remainingTime = burstTime;
            this->completionTime = 0;
            this->turnaroundTime = 0;
            this->waitingTime = 0;
        }

        // Getter methods (const - read-only, cannot modify object state)
        // Returns the unique Process ID identifier
        int getPID() const { return PID; }
        
        // Returns the time when this process arrives in the system
        int getArrivalTime() const { return arrivalTime; }
        
        // Returns the total CPU time required to complete this process
        int getBurstTime() const { return burstTime; }
        
        // Returns the priority level (lower number = higher priority)
        int getPriority() const { return priority; }
        
        // Returns the remaining CPU time still needed (changes during execution)
        int getRemainingTime() const { return remainingTime; }

        // Setter methods - modify process state during scheduling
        // Updates remaining time when a time quantum expires (used in Round Robin)
        void setRemainingTime(int time) { remainingTime = time; }
        
        // Records the completion time when process finishes execution
        void setCompletionTime(int time) { completionTime = time; }
        
        // Calculator methods - derive metrics from completion time and arrival time
        // Calculates turnaround time: how long from arrival to completion
        void calculateTurnaroundTime() { turnaroundTime = completionTime - arrivalTime; }
        
        // Calculates waiting time: total time waiting (turnaround - actual execution)
        void calculateWaitingTime() { waitingTime = turnaroundTime - burstTime; }
};

class Scheduler {
public:
    // FCFS - First Come First Served
    static vector<ExecutionSegment> FCFS(vector<Process>& processes) {
        vector<ExecutionSegment> execution;
        
        sort(processes.begin(), processes.end(),
             [](const Process& a, const Process& b) {
                 return a.getArrivalTime() < b.getArrivalTime();
             });

        int currentTime = 0;
        for (auto& p : processes) {
            if (currentTime < p.getArrivalTime()) {
                currentTime = p.getArrivalTime();
            }
            int startTime = currentTime;
            currentTime += p.getBurstTime();
            p.setCompletionTime(currentTime);
            p.calculateTurnaroundTime();
            p.calculateWaitingTime();
            
            execution.push_back({p.getPID(), startTime, currentTime});
        }
        
        return execution;
    }

    // SJF - Shortest Job First (Non-preemptive)
    // This algorithm selects the process with the shortest burst time that has arrived by the current time.
    // It is non-preemptive, meaning once a process starts, it runs to completion.
    static vector<ExecutionSegment> SJF(vector<Process>& processes) {
        vector<ExecutionSegment> execution;
        vector<bool> processed(processes.size(), false); // Track which processes have been completed
        int currentTime = 0; // Current simulation time
        int completed = 0; // Number of processes completed

        // Continue until all processes are completed
        while (completed < processes.size()) {
            int shortest = -1; // Index of the shortest job found
            int shortestBurst = INT_MAX; // Burst time of the shortest job

            // Find the process with the smallest burst time that has arrived and not yet processed
            for (int i = 0; i < processes.size(); i++) {
                if (!processed[i] && processes[i].getArrivalTime() <= currentTime &&
                    processes[i].getBurstTime() < shortestBurst) {
                    shortest = i;
                    shortestBurst = processes[i].getBurstTime();
                }
            }

            // If no process has arrived yet, advance time to the next arrival time
            if (shortest == -1) {
                for (int i = 0; i < processes.size(); i++) {
                    if (!processed[i]) {
                        currentTime = processes[i].getArrivalTime();
                        shortest = i;
                        break;
                    }
                }
            }

            // Execute the selected process
            processed[shortest] = true;
            int startTime = currentTime;
            currentTime += processes[shortest].getBurstTime();
            processes[shortest].setCompletionTime(currentTime);
            processes[shortest].calculateTurnaroundTime();
            processes[shortest].calculateWaitingTime();
            
            execution.push_back({processes[shortest].getPID(), startTime, currentTime});
            completed++;
        }
        
        return execution;
    }

    // Round Robin
    // This preemptive algorithm uses a time quantum. Each process gets a fixed time slice (quantum).
    // If a process doesn't finish in its quantum, it's preempted and placed back in the queue.
    // Processes are enqueued in arrival order initially.
    static vector<ExecutionSegment> RoundRobin(vector<Process>& processes, int timeQuantum) {
        vector<ExecutionSegment> execution;
        queue<int> q; // Queue to hold process indices
        vector<int> remainingTime(processes.size()); // Remaining burst time for each process
        
        // Initialize remaining times
        for (int i = 0; i < processes.size(); i++) {
            remainingTime[i] = processes[i].getBurstTime();
        }

        // Sort processes by arrival time to enqueue them in order
        sort(processes.begin(), processes.end(),
             [](const Process& a, const Process& b) {
                 return a.getArrivalTime() < b.getArrivalTime();
             });

        // Enqueue all processes initially (assuming they arrive at time 0 or later, but queue handles order)
        for (int i = 0; i < processes.size(); i++) {
            q.push(i);
        }

        int currentTime = 0;
        // Process the queue until empty
        while (!q.empty()) {
            int idx = q.front();
            q.pop();

            int startTime = currentTime;
            // If remaining time > quantum, execute for quantum and requeue
            if (remainingTime[idx] > timeQuantum) {
                currentTime += timeQuantum;
                remainingTime[idx] -= timeQuantum;
                execution.push_back({processes[idx].getPID(), startTime, currentTime});
                q.push(idx); // Requeue the process
            } else {
                // Execute for remaining time and complete the process
                currentTime += remainingTime[idx];
                remainingTime[idx] = 0;
                processes[idx].setCompletionTime(currentTime);
                processes[idx].calculateTurnaroundTime();
                processes[idx].calculateWaitingTime();
                execution.push_back({processes[idx].getPID(), startTime, currentTime});
            }
        }
        
        return execution;
    }

    // Priority Scheduling (Non-preemptive) - Lower priority number = higher priority
    // This algorithm selects the process with the highest priority (lowest number) that has arrived.
    // If withAging is true, priorities improve over time to prevent starvation.
    // Aging reduces priority by 1 every 'agingInterval' time units waited.
    // Tie-breaking: earlier arrival time, then smaller burst time.
    static vector<ExecutionSegment> PriorityScheduling(vector<Process>& processes, bool withAging = true) {
        vector<ExecutionSegment> execution;
        vector<bool> processed(processes.size(), false); // Track completed processes
        int currentTime = 0; // Current simulation time
        int completed = 0; // Number of processes completed
        // Aging parameters: every `agingInterval` time units a waiting process's
        // numeric priority is reduced by 1 (improves its priority since lower
        // number means higher priority). This prevents starvation of low-priority
        // processes.
        const int agingInterval = 5; // time units per priority improvement

        // Continue until all processes are completed
        while (completed < processes.size()) {
            int highest = -1; // Index of the highest priority process
            int highestPriority = INT_MAX; // Effective priority of the selected process

            // Find the process with the highest priority (lowest effective priority number)
            for (int i = 0; i < processes.size(); i++) {
                if (processed[i]) continue; // Skip already processed
                int at = processes[i].getArrivalTime();
                if (at > currentTime) continue; // Not arrived yet

                // Compute effective priority with aging if enabled
                int effectivePriority;
                if (withAging) {
                    int waitTime = currentTime - at; // Time spent waiting
                    int priorityDrop = waitTime / agingInterval; // Integer division for aging steps
                    effectivePriority = processes[i].getPriority() - priorityDrop;
                    if (effectivePriority < 0) effectivePriority = 0; // Clamp to prevent negative
                } else {
                    effectivePriority = processes[i].getPriority(); // No aging
                }

                // Select the process with the lowest effective priority value
                if (effectivePriority < highestPriority) {
                    highest = i;
                    highestPriority = effectivePriority;
                } else if (effectivePriority == highestPriority) {
                    // Tie-breaker: earlier arrival wins
                    if (at < processes[highest].getArrivalTime()) {
                        highest = i;
                    } else if (at == processes[highest].getArrivalTime() &&
                               processes[i].getBurstTime() < processes[highest].getBurstTime()) {
                        // If same arrival, smaller burst wins
                        highest = i;
                    }
                }
            }

            // If no process is ready, advance time to the next arrival
            if (highest == -1) {
                int nextArrival = INT_MAX;
                for (int i = 0; i < processes.size(); i++) {
                    if (!processed[i]) nextArrival = min(nextArrival, processes[i].getArrivalTime());
                }
                currentTime = nextArrival;
                continue; // Restart the loop with new time
            }

            // Execute the selected process to completion
            processed[highest] = true;
            int startTime = currentTime;
            currentTime += processes[highest].getBurstTime();
            processes[highest].setCompletionTime(currentTime);
            processes[highest].calculateTurnaroundTime();
            processes[highest].calculateWaitingTime();

            execution.push_back({processes[highest].getPID(), startTime, currentTime});
            completed++;
        }
        
        return execution;
    }
};

void displayResults(const vector<Process>& processes, const string& algorithmName) {
    cout << "\n" << string(80, '=') << endl;
    cout << "Algorithm: " << algorithmName << endl;
    cout << string(80, '=') << endl;
    cout << left << setw(8) << "PID" << setw(15) << "Arrival Time"
         << setw(12) << "Burst Time" << setw(18) << "Completion Time"
         << setw(15) << "Turnaround" << setw(12) << "Waiting" << endl;
    cout << string(80, '-') << endl;

    double avgTurnaround = 0, avgWaiting = 0, totalTime = 0;
    // Calculate averages and find the total time (maximum completion time)
    for (const auto& p : processes) {
        cout << left << setw(8) << p.getPID()
             << setw(15) << p.getArrivalTime()
             << setw(12) << p.getBurstTime()
             << setw(18) << p.completionTime
             << setw(15) << p.turnaroundTime
             << setw(12) << p.waitingTime << endl;
        avgTurnaround += p.turnaroundTime;
        avgWaiting += p.waitingTime;
        
        // Track the maximum completion time to calculate total execution time
        if (p.completionTime > totalTime) {
            totalTime = p.completionTime;
        }
    }

    avgTurnaround /= processes.size();
    avgWaiting /= processes.size();
    
    // Throughput: number of processes completed per unit time (processes / total time)
    double throughput = processes.size() / totalTime;

    cout << string(80, '-') << endl;
    cout << "Average Turnaround Time: " << fixed << setprecision(2) << avgTurnaround << endl;
    cout << "Average Waiting Time: " << fixed << setprecision(2) << avgWaiting << endl;
    cout << "Throughput: " << fixed << setprecision(2) << throughput << endl;
}

// Function to display Gantt Chart
void displayGanttChart(const vector<ExecutionSegment>& execution) {
    if (execution.empty()) return;
    
    cout << "\n" << string(80, '=') << endl;
    cout << "GANTT CHART VISUALIZATION" << endl;
    cout << string(80, '=') << endl;
    
    // Find the maximum time for scaling
    int maxTime = 0;
    for (const auto& seg : execution) {
        maxTime = max(maxTime, seg.endTime);
    }
    
    // Draw the timeline
    cout << "\nTimeline:" << endl;
    cout << "Time: ";
    for (int i = 0; i <= maxTime; i += 2) {
        cout << setw(3) << i;
    }
    cout << endl;
    
    // Draw Gantt chart bars
    cout << "      ";
    for (int i = 0; i < maxTime; i++) {
        cout << "-";
    }
    cout << endl;
    
    // Display each process segment
    for (const auto& seg : execution) {
        cout << "P" << seg.processID << "   |";
        
        // Print spaces before the process
        for (int i = 0; i < seg.startTime; i++) {
            cout << " ";
        }
        
        // Print the process block
        int duration = seg.endTime - seg.startTime;
        for (int i = 0; i < duration; i++) {
            cout << "=";
        }
        
        // Print process duration label
        cout << "(" << duration << ")";
        cout << endl;
    }
    
    cout << "      ";
    for (int i = 0; i < maxTime; i++) {
        cout << "-";
    }
    cout << endl;
    
    // Display time markers
    cout << "      ";
    for (int i = 0; i <= maxTime; i++) {
        if (i % 2 == 0) cout << i;
        else cout << " ";
    }
    cout << endl;
    
    // Display process details
    cout << "\nProcess Execution Details:" << endl;
    cout << left << setw(10) << "Process" << setw(12) << "Start Time" << setw(12) << "End Time" << setw(12) << "Duration" << endl;
    cout << string(46, '-') << endl;
    
    for (const auto& seg : execution) {
        cout << left << setw(10) << "P" + to_string(seg.processID)
             << setw(12) << seg.startTime
             << setw(12) << seg.endTime
             << setw(12) << (seg.endTime - seg.startTime) << endl;
    }
}

// Function to execute the selected scheduling algorithm
void executeScheduler(vector<Process>& processes, int choice) {
    vector<Process> tempProcesses = processes;
    vector<ExecutionSegment> execution;

    switch (choice) {
        case 1: {
            // Execute FCFS algorithm
            execution = Scheduler::FCFS(tempProcesses);
            displayResults(tempProcesses, "FCFS");
            displayGanttChart(execution);
            break;
        }
        case 2: {
            // Execute SJF algorithm
            execution = Scheduler::SJF(tempProcesses);
            displayResults(tempProcesses, "SJF");
            displayGanttChart(execution);
            break;
        }
        case 3: {
            // Execute Round Robin with user-defined time quantum
            int quantum;
            cout << "Enter time quantum for Round Robin: ";
            cin >> quantum;
            execution = Scheduler::RoundRobin(tempProcesses, quantum);
            displayResults(tempProcesses, "Round Robin (Quantum = " + to_string(quantum) + ")");
            displayGanttChart(execution);
            break;
        }
        case 4: {
            // Execute Priority Scheduling algorithm without aging
            execution = Scheduler::PriorityScheduling(tempProcesses, false);
            displayResults(tempProcesses, "Priority Scheduling (without aging)");
            displayGanttChart(execution);
            break;
        }
        case 5: {
            // Execute Priority Scheduling algorithm with aging
            execution = Scheduler::PriorityScheduling(tempProcesses, true);
            displayResults(tempProcesses, "Priority Scheduling (with aging)");
            displayGanttChart(execution);
            break;
        }
        default:
            cout << "Invalid choice! Please try again." << endl;
    }
}

int main() {
    // Processes can be provided interactively or the program can use a
    // built-in default set. Interactive input expects: PID Arrival Burst Priority
    vector<Process> processes;
    cout << "Use default process set? (y/n): ";
    char useDefault;
    cin >> useDefault;
    if (useDefault == 'y' || useDefault == 'Y') {
        processes = {
            Process(1, 3, 5, 8),
            Process(3, 7, 5, 2),
            Process(2, 9, 7, 1),
            Process(4, 8, 2, 9),
            Process(7, 4, 1, 3),
            Process(6, 3, 1, 1),
            Process(5, 1, 9, 6),
            Process(8, 7, 3, 5),
            Process(9, 1, 4, 2),
            Process(10, 9, 3, 5)
        };
    } else {
        int n;
        cout << "Enter number of processes: ";
        while (!(cin >> n) || n <= 0) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Please enter a positive integer: ";
        }
        cout << "Enter process details (PID Arrival Burst Priority) in the given order:" << endl;
        for (int i = 0; i < n; ++i) {
            int pid, at, bt, pr;
            cout << "Process " << (i + 1) << " - Enter: PID Arrival Burst Priority: ";
            while (!(cin >> pid >> at >> bt >> pr)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Enter: PID Arrival Burst Priority: ";
            }
            processes.emplace_back(pid, at, bt, pr);
        }
    }

    int choice;
    while (true) {
        cout << "\n" << string(80, '=') << endl;
        cout << "CPU SCHEDULING ALGORITHMS" << endl;
        cout << string(80, '=') << endl;
        cout << "1. FCFS (First Come First Served)" << endl;
        cout << "2. SJF (Shortest Job First)" << endl;
        cout << "3. Round Robin" << endl;
        cout << "4. Priority Scheduling" << endl;
        cout << "5. Priority Scheduling(with aging)" << endl;
        cout << "6. Exit" << endl;
        cout << string(80, '-') << endl;
        cout << "Enter your choice (1-6): ";
        cin >> choice;

        if (choice == 6) break;

        // Call the executeScheduler function with user choice
        executeScheduler(processes, choice);
    }

    cout << "\nThank you for using CPU Scheduler!" << endl;
    return 0;
}
