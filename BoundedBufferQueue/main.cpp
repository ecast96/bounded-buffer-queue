//
//  main.cpp
//  BoundedBufferQueue
//
//  Created by Erick Castro on 10/28/18.
//  CSCI 144 - Project 1

#include <cstdlib>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <sstream>
#include <unordered_map>
using namespace std;

const int MAX = 10;

class threadQueue {

    vector<thread> producers;
    vector<thread> consumers;
    
    mutex mtx;      // Lock used for main operation
    mutex coutMtx;  // Lock used for "cout" operations
    
    int counter = 0;
    int items[MAX];
    int front;
    int nextEmpty;
    
    int producerSleep;
    int consumerSleep;
    
    int producerIds = 1;
    int consumerIds = 1;
    unordered_map<thread::id, int> producerThreadIds;
    unordered_map<thread::id, int> consumerThreadIds;
    
public:
    threadQueue(){front = nextEmpty = 0;}
    ~threadQueue(){};
    
    void producerFunction();
    void consumerFunction();
    void coutWithLock(string str);
    
    // Sets sleep time limit given from user input
    void setSleepTimers(int p, int c){
        producerSleep = p;
        consumerSleep = c;
    }
    
    // Initializes threads
    // - calls producer/consumer functions
    // - terminates threads after completion of operations (not reached in this project's case)
    void startThreads(){
        for(int i = 0; i < 10; i++){
            coutMtx.lock();
            cout << "Producer " << i+1 << " created" << endl;
            coutMtx.unlock();
            producers.push_back(thread(&threadQueue::producerFunction, this));
        }

        for(int i = 0; i < 10; i++){
            coutMtx.lock();
            cout << "Consumer " << i+1 << " created" << endl;
            coutMtx.unlock();
            consumers.push_back(thread(&threadQueue::consumerFunction, this));
        }
        
        terminateThreads();
    }
    
    // Terminates threads
    void terminateThreads(){
        for(auto& thread : producers){
            coutMtx.lock();
            cout << "Ending producer thread: " << producerThreadIds[this_thread::get_id()] << endl;
            coutMtx.unlock();
            thread.join();
        }
        for(auto& thread : consumers){
            coutMtx.lock();
            cout << "Ending consumer thread: " << consumerThreadIds[this_thread::get_id()] << endl;
            coutMtx.unlock();
            thread.join();
        }
    }
    
    // Try to insert an item. If the queue
    // is full, return false; otherwise return true.
    bool tryInsert(int item){
        bool success = false;
        
        mtx.lock();
        if((nextEmpty - front) < MAX){
            items[nextEmpty % MAX] = item;
            nextEmpty++;
            success = true;
        }
        mtx.unlock();
        return success;
    }
    
    // Try to remove an item. If the queue is
    // empty, return false; otherwise return true
    bool tryRemove(int *item){
        bool success = false;
        
        mtx.lock();
        if(front < nextEmpty) {
            *item = items[front % MAX];
            front++;
            success = true;
        }
        mtx.unlock();
        return success;
    }

};

// Used to do "cout" operations without interrupts from other threads
void threadQueue::coutWithLock(string str){
    coutMtx.lock();
    cout << str << endl;
    coutMtx.unlock();
}

// Infinite loop that attempts to insert items
// - sleeps for a random number (0 - TP)
// - TP (producer sleep time limit) comes from user input #1
void threadQueue::producerFunction()
{
    producerThreadIds[this_thread::get_id()] = producerIds++;
    while(true){
        coutMtx.lock();
        cout << "Item #" << counter << " produced by thread " << producerThreadIds[this_thread::get_id()] << endl;
        coutMtx.unlock();
        tryInsert(counter++);
        this_thread::sleep_for(chrono::seconds(rand() % producerSleep));
    }
}

// Infinite loop that attempts to remove items
// - sleeps for a random number (0 - TC)
// - TP (producer sleep time limit) comes from user input #2
void threadQueue::consumerFunction()
{
    consumerThreadIds[this_thread::get_id()] = consumerIds++;
    int item;
    while(true){
        if(tryRemove(&item)){
            coutMtx.lock();
            cout << "Item #" << item << " consumed by thread " << consumerThreadIds[this_thread::get_id()] << endl;
            coutMtx.unlock();
        }
        else
            coutWithLock("Nothing there.");
        this_thread::sleep_for(chrono::seconds(rand() % consumerSleep));
    }
}



// Main program
// - Sets thread sleep time limits and starts threads
int main(int argc, const char * argv[]) {
    
    srand(time(NULL));
    int TP = atoi(argv[1]); // sleep time range limit for producer threads
    int TC = atoi(argv[2]); // sleep time range limit for consumer threads
    
    if(TP <= 0){
        cout << "Error: TP value needs to be an integer greater than 0" << endl;
        return 0;
    } else if(TC <= 0) {
        cout << "Error: TC value needs to be an integer greater than 0" << endl;
        return 0;
    }
    
    threadQueue queue;
    queue.setSleepTimers(TP, TC);
    queue.startThreads();
    
    return 0;
}
