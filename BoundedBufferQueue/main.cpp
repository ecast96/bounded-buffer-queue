//
//  main.cpp
//  BoundedBufferQueue
//
//  Created by Erick Castro on 10/28/18.
//

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
    
    mutex mtx;
    mutex coutMtx;
    
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
    
    void setSleepTimers(int p, int c){
        producerSleep = p;
        consumerSleep = c;
    }
    
    // Initializes threads
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
    
    // Terminates threads by joining them
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

void threadQueue::coutWithLock(string str){
    coutMtx.lock();
    cout << str << endl;
    coutMtx.unlock();
}

void threadQueue::producerFunction()
{
    producerThreadIds[this_thread::get_id()] = producerIds++;
    stringstream ss;
    while(true){
        coutMtx.lock();
        cout << "Item #" << counter << " produced by thread " << producerThreadIds[this_thread::get_id()] << endl;
        coutMtx.unlock();
        tryInsert(counter++);
        this_thread::sleep_for(chrono::seconds(rand() % producerSleep));
    }
}

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




int main(int argc, const char * argv[]) {
    srand(time(NULL));
    int TP = atoi(argv[1]); // sleep time range limit for producing threads
    int TC = atoi(argv[2]); // sleep time range limit for consuming threads
    
    threadQueue queue;
    queue.setSleepTimers(TP, TC);
    queue.startThreads();
    
    return 0;
}
