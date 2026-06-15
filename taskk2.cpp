#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <deque>
#include <string>
#include <sstream>
#include <algorithm>
using namespace std;

struct Task {
    string id;
    int burst, remaining;
    vector<string> memBlocks;
    size_t memIndex = 0;
};

struct CacheLevel {
    deque<string> blocks;
    int capacity;
    int latency;
};

bool isInCache(CacheLevel& c, const string& b) {
    return find(c.blocks.begin(), c.blocks.end(), b) != c.blocks.end();
}

void removeFromCache(CacheLevel& c, const string& b) {
    auto it = find(c.blocks.begin(), c.blocks.end(), b);
    if (it != c.blocks.end()) c.blocks.erase(it);
}

string addToCache(CacheLevel& c, const string& b) {
    string evicted = "";
    if ((int)c.blocks.size() >= c.capacity) {
        evicted = c.blocks.back();
        c.blocks.pop_back();
    }
    c.blocks.push_front(b);
    return evicted;
}

string dump(CacheLevel& c) {
    string out;
    for (size_t i = 0; i < c.blocks.size(); i++) {
        out += c.blocks[i];
        if (i + 1 < c.blocks.size()) out += ", ";
    }
    return out;
}

vector<Task> readTasks(const string& filename) {
    vector<Task> tasks;
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string w;
        Task t;
        ss >> w;       // "TASK"
        ss >> t.id;    // T1
        ss >> w;       // "BURST"
        ss >> t.burst;
        t.remaining = t.burst;
        ss >> w;       // "MEM"
        while (ss >> w) t.memBlocks.push_back(w);
        tasks.push_back(t);
    }
    return tasks;
}

int main() {
    CacheLevel L1{{}, 32, 4};
    CacheLevel L2{{}, 128, 12};
    CacheLevel L3{{}, 512, 40};

    vector<Task> tasks = readTasks("input.txt");

    deque<int> rq;
    for (size_t i = 0; i < tasks.size(); i++) rq.push_back(i);

    const int QUANTUM = 3;
    int cycle = 1;
    int tasksCompleted = 0;
    int ramAccesses = 0;

    while (!rq.empty()) {
        int idx = rq.front();
        rq.pop_front();
        Task& t = tasks[idx];

        int runCycles = min(QUANTUM, t.remaining);

        for (int c = 0; c < runCycles; c++) {
            string block = "NONE";
            if (!t.memBlocks.empty()) {
                block = t.memBlocks[t.memIndex % t.memBlocks.size()];
                t.memIndex++;
            }

            cout << "Cycle " << cycle << " - Running: " << t.id
                 << " Requesting: " << block << " ";

            if (isInCache(L1, block)) {
                cout << "L1: [" << dump(L1) << "] -> HIT "
                     << "L2: [" << dump(L2) << "] L3: [" << dump(L3) << "]" << endl;
            }
            else if (isInCache(L2, block)) {
                string l1Before = dump(L1);
                removeFromCache(L2, block);
                string evicted = addToCache(L1, block);
                string evInfo = evicted.empty() ? "" : " (" + evicted + " evicted)";
                cout << "L1: [" << l1Before << "] >> MISS "
                     << "L2: [" << dump(L2) << "] >> HIT (12 cycles) "
                     << "Promoting " << block << " -> L1 "
                     << "L1: [" << dump(L1) << "]" << evInfo
                     << " L2: [" << dump(L2) << "] L3: [" << dump(L3) << "]" << endl;
            }
            else if (isInCache(L3, block)) {
                string l1Before = dump(L1);
                removeFromCache(L3, block);
                string evicted = addToCache(L1, block);
                string evInfo = evicted.empty() ? "" : " (" + evicted + " evicted)";
                cout << "L1: [" << l1Before << "] >> MISS "
                     << "L2: [" << dump(L2) << "] >> MISS "
                     << "L3: [" << dump(L3) << "] >> HIT (40 cycles) "
                     << "Promoting " << block << " -> L1 "
                     << "L1: [" << dump(L1) << "]" << evInfo
                     << " L2: [" << dump(L2) << "] L3: [" << dump(L3) << "]" << endl;
            }
            else {
                ramAccesses++;
                string l1Before = dump(L1);
                string evicted = addToCache(L1, block);
                string evInfo = evicted.empty() ? "" : " (" + evicted + " evicted)";
                cout << "L1: [" << l1Before << "] >> MISS "
                     << "L2: [" << dump(L2) << "] >> MISS "
                     << "L3: [" << dump(L3) << "] >> MISS" << endl;
                cout << "Fetching from RAM "
                     << "L1: [" << dump(L1) << "]" << evInfo
                     << " L2: [" << dump(L2) << "] L3: [" << dump(L3) << "]" << endl;
            }

            t.remaining--;
            cycle++;
        }

        if (t.remaining > 0) {
            rq.push_back(idx);
        } else {
            tasksCompleted++;
        }
    }

    cout << "\n=== Final Results ===" << endl;
    cout << "Total Cycles: " << (cycle - 1) << endl;
    cout << "Tasks Completed: " << tasksCompleted << endl;
    cout << "Scheduler: Round Robin (quantum = " << QUANTUM << ")" << endl;
    cout << "RAM Accesses: " << ramAccesses << endl;

    return 0;
}