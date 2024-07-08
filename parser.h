#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <bits/stdc++.h>

using namespace std;

/*The parser file is responsible for reading and interpreting input data required for simulating various CPU scheduling algorithms. 
It initializes necessary global variables and structures to store the input data, which includes the scheduling operations, algorithms, and processes.*/


string operation; //operation: Stores the type of operation (likely to specify the scheduling algorithm)
int last_instant, process_count; //last_instant: The last time instant for the simulation. process_count: Number of processes to be simulated.
vector<pair<char, int>> algorithms; //algorithms: Stores scheduling algorithms and their parameters (e.g., quantum for Round Robin). 
vector<tuple<string,int,int>> processes; //processes: Stores the name, arrival time, and service time of each process.
vector<vector<char>>timeline; //timeline: A 2D vector to represent the timeline of process execution.
unordered_map<string,int>processToIndex; //processToIndex: Maps process names to their indices.


//Results stores for finish time, turn around time and normalised time

vector<int>finishTime;
vector<int>turnAroundTime;
vector<float>normTurn;


void parse_algorithms(string algorithm_chunk)
{
    stringstream stream(algorithm_chunk);
    while (stream.good())
    {
        string temp_str;
        getline(stream, temp_str, ',');
        stringstream ss(temp_str);
        getline(ss, temp_str, '-');
        char algorithm_id = temp_str[0];
        getline(ss, temp_str, '-');
        int quantum = temp_str.size() >= 1 ? stoi(temp_str) : -1;
        algorithms.push_back( make_pair(algorithm_id, quantum) );
    }
}
//process string is split into {process_name, process_arrival_time, and process_service_time}.
void parse_processes()
{
    string process_chunk, process_name;
    int process_arrival_time, process_service_time;
    for(int i=0; i<process_count; i++)
    {
        cin >> process_chunk;

        stringstream stream(process_chunk);
        string temp_str;
        getline(stream, temp_str, ',');
        process_name = temp_str;
        getline(stream, temp_str, ',');
        process_arrival_time = stoi(temp_str);
        getline(stream, temp_str, ',');
        process_service_time = stoi(temp_str);

        processes.push_back( make_tuple(process_name, process_arrival_time, process_service_time) );
        processToIndex[process_name] = i;
    }
}

void parse()
{
    string algorithm_chunk;
    cin >> operation >> algorithm_chunk >> last_instant >> process_count;
    parse_algorithms(algorithm_chunk);
    parse_processes();
    finishTime.resize(process_count);
    turnAroundTime.resize(process_count);
    normTurn.resize(process_count);
    timeline.resize(last_instant);
    for(int i=0; i<last_instant; i++)
        for(int j=0; j<process_count; j++)
            timeline[i].push_back(' ');
}


#endif // PARSER_H_INCLUDED
