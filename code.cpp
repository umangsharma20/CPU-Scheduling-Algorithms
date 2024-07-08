#include <bits/stdc++.h>
#include "parser.h"

#define all(v) v.begin(), v.end() //This is a macro to simplify iterating over entire containers.

using namespace std;

/** Global Constants **/
const string TRACE = "trace";
const string SHOW_STATISTICS = "stats";
const string ALGORITHMS[9] = {"", "FCFS", "RR-", "SPN", "SRT", "HRRN", "FB-1", "FB-2i", "AGING"};


/*  0 -> process name
    1 -> arrival time
    2 -> service time/ burst time     */
bool sortByServiceTime(const tuple<string, int, int> &a, const tuple<string, int, int> &b)
{   //Sorts tuples by service time (3rd element).
    return (get<2>(a) < get<2>(b));
}
bool sortByArrivalTime(const tuple<string, int, int> &a, const tuple<string, int, int> &b)
{   // Sorts tuples by arrival time (2nd element).
    return (get<1>(a) < get<1>(b));
}

bool descendingly_by_response_ratio(tuple<string, double, int> a, tuple<string, double, int> b)
{   //Sorts tuples by response ratio in descending order.
    return get<1>(a) > get<1>(b);
}

bool byPriorityLevel (const tuple<int,int,int>&a,const tuple<int,int,int>&b){
    // Sorts tuples by priority level, and by the third element if priority levels are equal.
    if(get<0>(a)==get<0>(b))
        return get<2>(a)> get<2>(b);
    return get<0>(a) > get<0>(b);
}

void clear_timeline()
{ //Initializes the timeline array with spaces.
    for(int i=0; i<last_instant; i++)
        for(int j=0; j<process_count; j++)
            timeline[i][j] = ' ';
}
//Utility functions to retrieve specific elements from the process tuples.
string getProcessName(tuple<string, int, int> &a)
{
    return get<0>(a);
}

int getArrivalTime(tuple<string, int, int> &a)
{
    return get<1>(a);
}

int getServiceTime(tuple<string, int, int> &a)
{
    return get<2>(a);
}

int getPriorityLevel(tuple<string, int, int> &a)
{
    return get<2>(a);
}

double calculate_response_ratio(int wait_time, int service_time)
{
    return (wait_time + service_time)*1.0 / service_time;
}

/*process was waiting in the timeline. 
If a process is running at a specific time, it is marked with '*', and if it is waiting, it is marked with '.' */
void fillInWaitTime(){
    for (int i = 0; i < process_count; i++)
    {
        int arrivalTime = getArrivalTime(processes[i]);
        for (int k = arrivalTime; k < finishTime[i]; k++)
        {
            if (timeline[k][i] != '*')
                timeline[k][i] = '.';
        }
    }
}
//Implements the FCFS scheduling algorithm, process that arrives first is executed first.
void firstComeFirstServe()
{
    int time = getArrivalTime(processes[0]);
    for (int i = 0; i < process_count; i++)
    {
        int processIndex = i;
        int arrivalTime = getArrivalTime(processes[i]);
        int serviceTime = getServiceTime(processes[i]);

        finishTime[processIndex] = (time + serviceTime);
        turnAroundTime[processIndex] = (finishTime[processIndex] - arrivalTime);
        normTurn[processIndex] = (turnAroundTime[processIndex] * 1.0 / serviceTime);//type coversion to integer

        for (int j = time; j < finishTime[processIndex]; j++)
            timeline[j][processIndex] = '*';
        for (int j = arrivalTime; j < time; j++)
            timeline[j][processIndex] = '.';
        time += serviceTime;
    }
}
//Implements the Round Robin scheduling algorithm with a specified time quantum.
void roundRobin(int originalQuantum)
{
    queue<pair<int,int>>q; // hold pairs of process index and remaining service time.
    int j=0; // tracks the index of the next process to be added to the queue.
    if(getArrivalTime(processes[j])==0){ // first process is added to the queue is arrives at time 0.
        q.push(make_pair(j,getServiceTime(processes[j])));
        j++;
    }
    int currentQuantum = originalQuantum;
    // main loop
    for(int time =0;time<last_instant;time++){
        if(!q.empty()){
            int processIndex = q.front().first;
            q.front().second = q.front().second-1;//decrement of remaining service time by 1 for each time unit reflects the actual progress of the process in using CPU time. It allows the algorithm to keep track of how much more CPU time each process needs before it completes, ensuring that each process gets its fair share of CPU time in a cyclic manner.
            int remainingServiceTime = q.front().second;
            int arrivalTime = getArrivalTime(processes[processIndex]);
            int serviceTime = getServiceTime(processes[processIndex]);
            currentQuantum--;
            timeline[time][processIndex]='*';

            // new processes arrival to the queue
            while(j<process_count && getArrivalTime(processes[j])==time+1){
                q.push(make_pair(j,getServiceTime(processes[j])));
                j++;
            }

            //quantum expires and the process is completed
            if(currentQuantum==0 && remainingServiceTime==0){
                finishTime[processIndex]=time+1;
                turnAroundTime[processIndex] = (finishTime[processIndex] - arrivalTime);
                normTurn[processIndex] = (turnAroundTime[processIndex] * 1.0 / serviceTime);//type coversion to integer
                currentQuantum=originalQuantum;
                q.pop();
            //quantum expires but the process is not completed 
            }else if(currentQuantum==0 && remainingServiceTime!=0){
                q.pop();
                q.push(make_pair(processIndex,remainingServiceTime));
                currentQuantum=originalQuantum;
            //quantum has not expired but the process is completed
            }else if(currentQuantum!=0 && remainingServiceTime==0){
                finishTime[processIndex]=time+1;
                turnAroundTime[processIndex] = (finishTime[processIndex] - arrivalTime);
                normTurn[processIndex] = (turnAroundTime[processIndex] * 1.0 / serviceTime);//type coversion to integer
                q.pop();
                currentQuantum=originalQuantum;
            }
        }
        //new processes arrival to the queue again
        while(j<process_count && getArrivalTime(processes[j])==time+1){
            q.push(make_pair(j,getServiceTime(processes[j])));
            j++;
        }
    }
    fillInWaitTime();
}

/*Implements the Shortest Job Next (SJN) scheduling algorithm, i.e, Shortest Job First (SJF) in its non-preemptive form
process with the shortest service/burst time from the set of processes that have arrived and are ready to execute. */
void shortestProcessNext()
{
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq; // min heap{service time and index}, queue order processes based on shortest burst/service time first(At top)
    int j = 0;
    for (int i = 0; i < last_instant; i++)
    {   //Add Processes to the Queue Based on Arrival Time
        while(j<process_count && getArrivalTime(processes[j]) <= i){
            pq.push(make_pair(getServiceTime(processes[j]), j));
            j++;
        }
        if (!pq.empty()) // means some processes are in the queue ready to be executed
        {
            int processIndex = pq.top().second; //the process with the shortest service/burst time (top of the priority queue).
            int arrivalTime = getArrivalTime(processes[processIndex]);
            int serviceTime = getServiceTime(processes[processIndex]);
            pq.pop();

            // filling the timeline
            int temp = arrivalTime;
            for (; temp < i; temp++)
                timeline[temp][processIndex] = '.';

            temp = i;
            for (; temp < i + serviceTime; temp++)
                timeline[temp][processIndex] = '*';

            finishTime[processIndex] = (i + serviceTime);
            turnAroundTime[processIndex] = (finishTime[processIndex] - arrivalTime);
            normTurn[processIndex] = (turnAroundTime[processIndex] * 1.0 / serviceTime); //type coversion to integer
            i = temp - 1;
        }
    }
}

//Implements the Shortest Remaining Time(SRT)scheduling algorithm,process with the shortest remaining time is always executed next
void shortestRemainingTime()
{
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq; // priority queue i.e,min-Heap{remaining service time, process index}, ensures that process with the shortest remainig time executes first(At top)
    int j = 0;
    for (int i = 0; i < last_instant; i++)
    {  
        //processes arriving at the current time unit (i) are added to the priority queue
        while(j<process_count &&getArrivalTime(processes[j]) == i){
            pq.push(make_pair(getServiceTime(processes[j]), j));
            j++;
        }
        if (!pq.empty())
        {   //process with the shortest remaining time is selected and that is always at the top.
            int processIndex = pq.top().second;
            int remainingTime = pq.top().first;
            pq.pop();
            int serviceTime = getServiceTime(processes[processIndex]);
            int arrivalTime = getArrivalTime(processes[processIndex]);
            timeline[i][processIndex] = '*'; // timeLine updated

            if (remainingTime == 1) // process finished
            {
                finishTime[processIndex] = i + 1;
                turnAroundTime[processIndex] = (finishTime[processIndex] - arrivalTime);
                normTurn[processIndex] = (turnAroundTime[processIndex] * 1.0 / serviceTime);  //type coversion to integer
            }
            else
            {//If the remaining time is more than 1, the process is reinserted into the priority queue with its remaining time decremented by 1.
                pq.push(make_pair(remainingTime - 1, processIndex));
            }
        }
    }
    fillInWaitTime(); //update the timeline
}

/*Implement of HRRN algorithm, that selects the next process to execute based on the highest response ratio
Response ratio = (waiting time + service time)/ service time   */
void highestResponseRatioNext()
{
    // Vector of tuple <process_name, process_response_ratio, time_in_service> for processes that are in the ready queue
    vector<tuple<string, double, int>> present_processes;
    int j=0;
    for (int current_instant = 0; current_instant < last_instant; current_instant++)
    {   // adding process to the ready queue with initial ratio of 1.0 and time in service of 0. 
        while(j<process_count && getArrivalTime(processes[j])<=current_instant){
            present_processes.push_back(make_tuple(getProcessName(processes[j]), 1.0, 0));
            j++;
        }
        // Calculate response ratio for every process
        for (auto &proc : present_processes)
        {
            string process_name = get<0>(proc); //// Get the name of the process from the tuple
            int process_index = processToIndex[process_name]; // Get the index of the process
            int wait_time = current_instant - getArrivalTime(processes[process_index]);// Calculate the waiting time
            int service_time = getServiceTime(processes[process_index]);// Get the service time (burst time)
            get<1>(proc) = calculate_response_ratio(wait_time, service_time);
        }

        // Sort present processes by highest to lowest response ratio(descending order).
        sort(all(present_processes), descendingly_by_response_ratio);

        //Execute the process with the highest response ratio
        if (!present_processes.empty())
        {
            int process_index = processToIndex[get<0>(present_processes[0])];
            //Execute the process until it completes its service time or until the last_instant is reached.
            while(current_instant<last_instant && get<2>(present_processes[0]) != getServiceTime(processes[process_index])){
                timeline[current_instant][process_index]='*';
                current_instant++;
                get<2>(present_processes[0])++;
            }
            current_instant--;
            present_processes.erase(present_processes.begin()); // remove the completed process from ready queue
            finishTime[process_index] = current_instant + 1; // increment by 1 bcoz it is decrement by 1 after complete the process 
            turnAroundTime[process_index] = finishTime[process_index] - getArrivalTime(processes[process_index]);
            normTurn[process_index] = (turnAroundTime[process_index] * 1.0 / getServiceTime(processes[process_index]));
        }
    }
    fillInWaitTime();// timeline update
}


/*Implements a basic form of the Feedback Queue Scheduling algorithm, which dynamically adjusts the priority levels of processes based on their process execution time..
multi-level feedback queue (MLFQ) approach, where processes that consume more CPU time are gradually demoted to lower-priority levels.*/
void feedbackQ1()
{
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq; //pair of priority level and process index, return the smallest element first
    unordered_map<int,int>remainingServiceTime; //map from process index to the remaining service time
    int j=0;
    //add process at arrival time 0
    if(getArrivalTime(processes[0])==0){
        pq.push(make_pair(0,j)); //first process arrives at time 0, it is added to the priority queue with priority level 0(highest priority level).
        remainingServiceTime[j]=getServiceTime(processes[j]);// Initialize its remaining service time.
        j++;
    }
    for(int time =0;time<last_instant;time++){
        if(!pq.empty()){
            // Extract the process with the highest priority (smallest priority level).
            int priorityLevel = pq.top().first;
            int processIndex =pq.top().second;
            int arrivalTime = getArrivalTime(processes[processIndex]);
            int serviceTime = getServiceTime(processes[processIndex]);
            pq.pop();

            //Add any new processes arriving at the next time unit (time + 1) to the priority queue.
            while(j<process_count && getArrivalTime(processes[j])==time+1){
                    pq.push(make_pair(0,j));
                    remainingServiceTime[j]=getServiceTime(processes[j]);
                    j++;
            }
            remainingServiceTime[processIndex]--;
            timeline[time][processIndex]='*'; 
            if(remainingServiceTime[processIndex]==0){ //Check if Process is Completed(remaining service time is 0)
                finishTime[processIndex]=time+1;
                turnAroundTime[processIndex] = (finishTime[processIndex] - arrivalTime);
                normTurn[processIndex] = (turnAroundTime[processIndex] * 1.0 / serviceTime);
            }else{ 
                //If the process is not completed(time quantum expire), re-add it to the priority queue,Increase its priority level by 1 if there are other processes in the queue(otherwise don't) -- preemption
                if(pq.size()>=1)
                    pq.push(make_pair(priorityLevel+1,processIndex));//Processes that consume more CPU time are gradually moved to lower priority queues. This is achieved by incrementing the priority level (priorityLevel + 1).
                else
                    pq.push(make_pair(priorityLevel,processIndex));
            }
        }
        //Add New Processes Arriving at Current Time + 1 (or next time)
        while(j<process_count && getArrivalTime(processes[j])==time+1){
                pq.push(make_pair(0,j));// Add new process with priority level 0
                remainingServiceTime[j]=getServiceTime(processes[j]);
                j++;
        }
    }
    fillInWaitTime(); // timeline update
}


/*Implements a variation of the Feedback Queue Scheduling algorithm known as the exponential quantum multi-level feedback queue.
This approach uses dynamic time quanta that double with each lower priority level, ensuring that higher priority processes get smaller quanta and lower priority processes get larger quanta.*/
void feedbackQ2i()
{
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq; //pair of priority level and process index,return the smallest element first
    unordered_map<int,int>remainingServiceTime; //map from process index to the remaining service time
    int j=0;

    if(getArrivalTime(processes[0])==0){
        pq.push(make_pair(0,j)); // Add the first process to the priority queue with priority level 0.
        remainingServiceTime[j]=getServiceTime(processes[j]);// Initialize its remaining service time.
        j++;
    }
    for(int time =0;time<last_instant;time++){
        if(!pq.empty()){
            // Extract the process with the highest priority (smallest priority level).
            int priorityLevel = pq.top().first;
            int processIndex =pq.top().second;
            int arrivalTime = getArrivalTime(processes[processIndex]);
            int serviceTime = getServiceTime(processes[processIndex]);
            pq.pop();
            // Add any new processes arriving at the current or next time unit.
            while(j<process_count && getArrivalTime(processes[j])<=time+1){
                    pq.push(make_pair(0,j));// Add new process with priority level 0(Highest priority)
                    remainingServiceTime[j]=getServiceTime(processes[j]);
                    j++;
            }
             // Calculate the current time quantum based on the priority level (2^priorityLevel).
            int currentQuantum = pow(2,priorityLevel);//which means higher priority processes get shorter time quanta, while lower priority processes get longer quanta.
            int temp = time;
            // Execute the process for the duration of the time quantum or until it finishes
            while(currentQuantum && remainingServiceTime[processIndex]){
                currentQuantum--;
                remainingServiceTime[processIndex]--;
                timeline[temp++][processIndex]='*';
            }

            if(remainingServiceTime[processIndex]==0){ // Check if the process has completed.
                finishTime[processIndex]=temp;
                turnAroundTime[processIndex] = (finishTime[processIndex] - arrivalTime);
                normTurn[processIndex] = (turnAroundTime[processIndex] * 1.0 / serviceTime);
            }else{ // If process not completed, re-add it to the priority queue with adjusted priority level.
                if(pq.size()>=1)
                    pq.push(make_pair(priorityLevel+1,processIndex));// Increment priority level.
                else
                    pq.push(make_pair(priorityLevel,processIndex));// Keep the same priority level if it's the only process.
            }
            time = temp-1;
        }
        //Add New Processes Arriving at Current Time + 1 (or next time)
        while(j<process_count && getArrivalTime(processes[j])<=time+1){
                pq.push(make_pair(0,j));// Add new process with priority level 0
                remainingServiceTime[j]=getServiceTime(processes[j]);// Initialize remaining service time.
                j++;
        }
    }
    fillInWaitTime(); // timeline update
}
/*Implements a priority scheduling algorithm with aging,prevent starvation of lower-priority processes.
By gradually increases the priority of waiting processes over time to ensure they eventually get executed.*/
void aging(int originalQuantum)
{
    vector<tuple<int,int,int>>v; //tuple of priority level, process index and total waiting time
    int j=0,currentProcess=-1;
    // Main loop iterating through each time unit
    for(int time =0;time<last_instant;time++){
        // Adding new processes that arrive at the current time
        while(j<process_count && getArrivalTime(processes[j])<=time){
            v.push_back(make_tuple(getPriorityLevel(processes[j]),j,0));
            j++;
        }
        // Aging mechanism to increment priority levels and waiting times
        for(int i=0;i<v.size();i++){
            if(get<1>(v[i])==currentProcess){
                get<2>(v[i])=0;// Reset waiting time for current process
                get<0>(v[i])=getPriorityLevel(processes[currentProcess]); // Reset priority for current process
            }
            else{
                get<0>(v[i])++; // Increment priority level for aging
                get<2>(v[i])++; // Increment waiting time
            }
        }
        // Sort processes by priority level (higher priority comes first)
        sort(v.begin(),v.end(),byPriorityLevel);
        // Select the process with the highest priority
        currentProcess=get<1>(v[0]);
        int currentQuantum = originalQuantum;
        // Execute the selected process for the given quantum or until time limit reaches
        while(currentQuantum-- && time<last_instant){
            timeline[time][currentProcess]='*';
            time++;
        }
        time--;
    }
    fillInWaitTime();
}

void printAlgorithm(int algorithm_index)
{
    int algorithm_id = algorithms[algorithm_index].first - '0';
    if(algorithm_id==2)
        cout << ALGORITHMS[algorithm_id] <<algorithms[algorithm_index].second <<endl;
    else
        cout << ALGORITHMS[algorithm_id] << endl;
}

void printProcesses()
{
    cout << "Process    ";
    for (int i = 0; i < process_count; i++)
        cout << "|  " << getProcessName(processes[i]) << "  ";
    cout << "|\n";
}
void printArrivalTime()
{
    cout << "Arrival    ";
    for (int i = 0; i < process_count; i++)
        printf("|%3d  ",getArrivalTime(processes[i]));
    cout<<"|\n";
}
void printServiceTime()
{
    cout << "Service    |";
    for (int i = 0; i < process_count; i++)
        printf("%3d  |",getServiceTime(processes[i]));
    cout << " Mean|\n";
}
void printFinishTime()
{
    cout << "Finish     ";
    for (int i = 0; i < process_count; i++)
        printf("|%3d  ",finishTime[i]);
    cout << "|-----|\n";
}
void printTurnAroundTime()
{
    cout << "Turnaround |";
    int sum = 0;
    for (int i = 0; i < process_count; i++)
    {
        printf("%3d  |",turnAroundTime[i]);
        sum += turnAroundTime[i];
    }
    if((1.0 * sum / turnAroundTime.size())>=10)
		printf("%2.2f|\n",(1.0 * sum / turnAroundTime.size()));
    else
	 	printf(" %2.2f|\n",(1.0 * sum / turnAroundTime.size()));
}

void printNormTurn()
{
    cout << "NormTurn   |";
    float sum = 0;
    for (int i = 0; i < process_count; i++)
    {
        if( normTurn[i]>=10 )
            printf("%2.2f|",normTurn[i]);
        else
            printf(" %2.2f|",normTurn[i]);
        sum += normTurn[i];
    }

    if( (1.0 * sum / normTurn.size()) >=10 )
        printf("%2.2f|\n",(1.0 * sum / normTurn.size()));
	else
        printf(" %2.2f|\n",(1.0 * sum / normTurn.size()));
}
void printStats(int algorithm_index)
{
    printAlgorithm(algorithm_index);
    printProcesses();
    printArrivalTime();
    printServiceTime();
    printFinishTime();
    printTurnAroundTime();
    printNormTurn();
}

void printTimeline(int algorithm_index)
{
    for (int i = 0; i <= last_instant; i++)
        cout << i % 10<<" ";
    cout <<"\n";
    cout << "------------------------------------------------\n";
    for (int i = 0; i < process_count; i++)
    {
        cout << getProcessName(processes[i]) << "     |";
        for (int j = 0; j < last_instant; j++)
        {
            cout << timeline[j][i]<<"|";
        }
        cout << " \n";
    }
    cout << "------------------------------------------------\n";
}

void execute_algorithm(char algorithm_id, int quantum,string operation)
{
    switch (algorithm_id)
    {
    case '1':
        if(operation==TRACE)cout<<"FCFS  ";
        firstComeFirstServe();
        break;
    case '2':
        if(operation==TRACE)cout<<"RR-"<<quantum<<"  ";
        roundRobin(quantum);
        break;
    case '3':
        if(operation==TRACE)cout<<"SPN   ";
        shortestProcessNext();
        break;
    case '4':
        if(operation==TRACE)cout<<"SRT   ";
        shortestRemainingTime();
        break;
    case '5':
        if(operation==TRACE)cout<<"HRRN  ";
        highestResponseRatioNext();
        break;
    case '6':
        if(operation==TRACE)cout<<"FB-1  ";
        feedbackQ1();
        break;
    case '7':
        if(operation==TRACE)cout<<"FB-2i ";
        feedbackQ2i();
        break;
    case '8':
        if(operation==TRACE)cout<<"Aging ";
        aging(quantum);
        break;
    default:
        break;
    }
}
/*FCFS -> O(n)
RR -> O(n * q) 
SPN -> O(n^2)
SRT -> O(n^2)
HRRN -> O(n^2)
FB-1 and FB -2i -> O(n* log n)
n is number of processes, and q is the number of time quanta required for the longest job to complete.*/

int main()
{
    parse();
    for (int idx = 0; idx < (int)algorithms.size(); idx++)
    {
        clear_timeline();
        execute_algorithm(algorithms[idx].first, algorithms[idx].second,operation);
        if (operation == TRACE)
            printTimeline(idx);
        else if (operation == SHOW_STATISTICS)
            printStats(idx);
        cout << "\n";
    }
    cout<<"Successfully Generated!"<<endl;
    return 0;
}
