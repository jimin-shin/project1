//
//  main.cpp
//  project1
//
//  Created by Jimin Shin on 2/3/21.
//  Copyright © 2021 Jimin Shin. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "disk.h"
#include "thread.h"
#include <vector>
#include <queue>

#ifdef __APPLE__

mutex mutex1;
cv cv1;
cv cv2;
std::vector<std::ifstream*> ifs;
std::vector<int> disk_queue;
std::vector<int> req_queue;
int max;
int reqs;

void request(void *arg) {
    int num = *(int*) arg;
    
    int temp;
    std::vector<int> file;
    while (*ifs[num] >> temp) {
        file.push_back(temp);
    }
    
    int count = 0;
    
    mutex1.lock();
    
    while (count < file.size()) {
        
        while (disk_queue.size() == max) {
            cv1.wait(mutex1);
        }
        
        bool in_queue = false;
        for (int i = 0; i < req_queue.size(); ++i) {
            if (req_queue[i] == num) {
                in_queue = true;
            }
        }
        
        if (in_queue) {
            cv1.wait(mutex1);
        }
        
        else if (disk_queue.size() < max) {
            disk_queue.push_back(file[count]);
            req_queue.push_back(num);
            print_request(num, file[count]);
            cv2.signal();
            ++count;
            
            if (count == file.size()) {
                --reqs;
                mutex1.unlock();
                return;
            }
            
            cv1.wait(mutex1);
        }
        
    }
    
    --reqs;
    mutex1.unlock();
    
}

void service(void *arg) {
//    std::vector<int>* temp = (std::vector<int>*) arg;
    int max_disk_queue = *(int*) arg;
    max = max_disk_queue;
    int cur = 0;
    
    mutex1.lock();
    for (int i = 0; i < ifs.size(); ++i) {
        thread((thread_startfunc_t) request, (void *) &i);
    }
    
    while (!(disk_queue.size() == 0 && reqs == 0) ) {
        if (reqs < max_disk_queue) {
            max = reqs;
        }
        
        while (disk_queue.size() < max) {
            cv2.wait(mutex1);
        }
        
        int closest = disk_queue[0];
        int index = 0;
        for (int i = 1; i < disk_queue.size(); ++i) {
            if (closest - cur > disk_queue[i] - cur) {
                closest = disk_queue[i];
                index = i;
            }
        }
        print_service(index, closest);
        cur = closest;
        disk_queue.erase(disk_queue.begin() + index);
        req_queue.erase(req_queue.begin() + index);
        
        cv1.broadcast();
    }
    
    mutex1.unlock();
    
    
    
}

int main(int argc, const char** argv) {
    
    // Redirect input
    if (!freopen("/Users/jiminshin/Desktop/eecs281/project2/P2A-samples-Mac-Unix/project2a/project2a/Small-input-PR.txt", "r", stdin)) {
        std::cerr << "freopen() failed, file not found" << std::endl;
        exit(1);
    } // if
    #endif
    
    
    int max_disk_queue = atoi(argv[1]);
    
    for (int i = 2; i < argc; ++i) {
        // input_file per requester
        std::ifstream input(argv[i]);
        if (!input.is_open()) {
            std::cout << "File cannot be opened.\n";
            exit(1);
        }
        ifs.push_back(&input);
    }
    
    reqs = argc - 2;
    
    cpu::boot((thread_startfunc_t) service, (void *) &max_disk_queue, 0);
    
    return 0;
}
