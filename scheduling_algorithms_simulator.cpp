//simulates the execution of different CPU scheduling
//algorithms (NP, PP, SJF, SRTF)


#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <deque>
#include <cmath>

#include <stdio.h>
#include <stdlib.h>

using namespace std;

struct process {
    int id;
    int arrival;
    int burst;
    int timeLeft;
    int priority;
    int waitTime;
};

int simulate (list<process> &processlist, deque<process> readyQueue, bool prioritize, bool preemption);

int main(int argc, char *argv[]) {
    if(argc != 3){
        cout << "please provide only a valid input file and scheduling algorithm\n",stderr;
        abort();
    }

	ifstream file(argv[1]);
	if(!file){
		cout << "unable to open input file\n", stderr;
        abort();
    }

    bool prioritize = false;
    bool preempt = false;
    string algorithm = argv[2];
    if (algorithm != "NP" && algorithm != "PP" && algorithm != "SJF" && algorithm != "SRTF" ){
        cout << "invalid input for scheduling algorithm\n",stderr;
        abort();
    }
    if (algorithm == "NP" || algorithm == "PP"){
        prioritize = true;
    }
    if (algorithm == "SRTF" || algorithm ==  "PP") {
        preempt = true;
    }

    int s = 0;
    vector<process> p;
	int id, arrival, burst, priority;
	while(file >> id >> arrival >> burst >> priority){
        p.resize(s + 1);
		p[s].id = id;
		p[s].arrival = arrival;
		p[s].burst = burst;
		p[s].timeLeft = p[s].burst;
		p[s].priority = priority;
		p[s].waitTime = 0;
        s += 1;
	}

    list<process> orderedList;
    int current = 0;
    int minimum = p[0].arrival;
    while (!p.empty()){
        for (int i = 1; i < p.size(); i++){
            if (p[i].arrival < minimum){
                minimum = p[i].arrival;
                current = i;
            } else if (p[i].arrival == minimum) {
                if (p[i].id < p[current].id){
                    current = i;
                }
            }
        }

        orderedList.push_back(p[current]);
        p.erase(p.begin() + current);
        minimum = p[0].arrival;
        current = 0;
    }

    deque<process> readyQueue;
    simulate(orderedList, readyQueue, prioritize, preempt);
}

int simulate (list<process> &processList, deque<process> readyQueue, bool prioritize, bool preemption) {
    deque<process>::iterator itr = readyQueue.begin();
    int totalIdle = 0;
    int totalBurst = 0;
    int totalWait = 0;
    if (processList.front().arrival != 0){
        totalIdle += processList.front().arrival;
        cout << "Time " << 0 << " idle" << endl;
    }
    int currentTime = processList.front().arrival;
    int worst_case_wait = 0;
    int finishedProcesses = 0;
    int lastPID = -1;
    int rQ_size = 0;
    int sessionStart = 0;

    while (!processList.empty() || !readyQueue.empty()) {
        while (!processList.empty() && currentTime >= processList.front().arrival) {
        //Inserts a process into the ready queue in the correct position
            process next = processList.front();
            processList.pop_front();
            if(prioritize) {
            //Higher priority processes are favored
                if (readyQueue.empty()){
                    readyQueue.push_back(next);
                } else {
                    rQ_size = readyQueue.size();
                    for (int i = 0; i < rQ_size; i++){
                        if (readyQueue[i].priority > next.priority){
                            itr = readyQueue.begin()+ i;
                            readyQueue.insert(itr, next);
                            itr = readyQueue.begin();
                            break;
                        } else if (readyQueue[rQ_size-1].priority < next.priority){
                            readyQueue.push_back(next);//
                            break;
                        }
                    }
                }
            } else {
            //Processes with less time remaining to finish are favored
                if (readyQueue.empty()){
                    readyQueue.push_back(next);
                } else {
                    rQ_size = readyQueue.size();
                    for (int i = 0; i < rQ_size; i++){
                        if (readyQueue[i].timeLeft > next.timeLeft){
                            itr = readyQueue.begin()+ i;
                            readyQueue.insert(itr, next);
                            itr = readyQueue.begin();
                            break;
                        } else if (readyQueue[i].timeLeft < next.timeLeft && rQ_size == i + 1){
                            readyQueue.push_back(next);
                            break;
                        }
                    }
                }
            }
        }

        if (readyQueue.empty()){
        //No processes are ready, the CPU is idle
            totalIdle += processList.front().arrival - currentTime;
            cout << "Time " << processList.front().arrival - currentTime << " idle" << endl;
            currentTime = processList.front().arrival;
            lastPID = -1;
        } else if (preemption){
        // Processes utilize the CPU, higher priority processes can interrupt
            process &q = readyQueue[0];
            sessionStart = currentTime;
            while((processList.empty() || currentTime != processList.front().arrival) && q.timeLeft != 0){
                currentTime++;
                q.timeLeft--;
            }
            if (q.timeLeft == 0){
            // The current process is completed
                q.waitTime = currentTime - q.burst - q.arrival;
                totalWait += q.waitTime;
                if (q.waitTime > worst_case_wait) {
                    worst_case_wait = q.waitTime;
                }
                totalBurst += q.burst;
                if (q.id != lastPID){
                    cout << "Time " << sessionStart << " ";
                    cout << "Process " << q.id << endl;
                }
                lastPID = q.id;
                readyQueue.pop_front();
                finishedProcesses++;
            } else if ((currentTime == processList.front().arrival) && !processList.empty()){
            // A new process has arrived, return to loop and add to ready queue
                cout << "Time " << sessionStart << " ";
                cout << "Process " << q.id << endl;
                lastPID = q.id;
            }

        } else {
        // Processes utilize the CPU, processes can not be interrupted
            process &q = readyQueue[0];
            currentTime += q.timeLeft;
            q.timeLeft = 0;
            cout << "Time " << currentTime - q.burst << " ";
            cout << "Process " << q.id << endl;
            q.waitTime = currentTime - q.burst - q.arrival;
            totalWait += q.waitTime;
            if (q.waitTime > worst_case_wait) {
                worst_case_wait = q.waitTime;
            }
            totalBurst += q.burst;
            readyQueue.pop_front();
            finishedProcesses++;
        }
    }

    cout << "CPU Utilization: ";
    cout << round(((double)currentTime - (double)totalIdle) / (double)currentTime * 100);
    cout << "%" << endl;
    double averageWait = round(100 * (double)totalWait / (double)finishedProcesses) / 100;
    cout << "Average wait time: " << averageWait << endl;
    cout << "Worst-case waiting time: " << worst_case_wait << endl;
}
