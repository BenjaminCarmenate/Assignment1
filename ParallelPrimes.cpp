#include <iostream>
#include <thread>
#include <stdio.h>
#include <cstring>
#include <mutex>
#include <vector>
#include <math.h>

std::mutex runLock;

class lockableCounter{
    public:
        lockableCounter(){
            count = 2;
        }
        int getCounter(){
            std::unique_lock<std::mutex> mtx_lock(counterLock);
            int a = this->count;
            this->count++;
            mtx_lock.unlock();
            return a;
        }
    private:
        std::mutex counterLock;
        long long int count;
};

class sharedList{
    public:
        sharedList(int size){
            primeList = std::vector<bool>(size, true);
            this->size = size;
            numPrimes = size-2;         
        }
        int getSize(){
            return this->size;
        }
        bool getVal(int i){
            std::unique_lock<std::mutex> lock(writeLock);
            bool temp = false;
            if(this->primeList[i])
                temp = true;
            return temp;          
        }
        void setFalse(int i)
        {
            std::unique_lock<std::mutex> wl(writeLock);
            std::unique_lock<std::mutex> rl(readLock);
            primeList[i] = false;
            numPrimes--;
        }
        int getNumPrimes(){
            return this->numPrimes;
        }
    private:
        std::mutex readLock, writeLock;
        std::vector<bool> primeList;
        long long int size;
        long long int numPrimes;
};

void primeNumberFinder(lockableCounter *c, sharedList *list){
    long long int i; 
    while((i = c->getCounter()) <= sqrt(list->getSize()))
    {
        if(!list->getVal(i))
            continue;
        runLock.lock();
        if(list->getVal(i))
        {
            for(long long int j = i*i; j < list->getSize(); j+=i){
                if(list->getVal(j))
                    list->setFalse(j);
            }
        }
        runLock.unlock();
    }   
}

int main(int argc, char**argv){

    lockableCounter *counter = new lockableCounter();
    sharedList *list = new sharedList(100000000);

    std::vector<std::thread> threads;
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 8; i++)
    {
        threads.emplace_back(std::thread(primeNumberFinder, counter, list));
    }
    for(int i = 0; i < 8; i++)
    {
        threads[i].join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    
    int count = 0;
    long long int sum = 0;
    std::vector<long long int> topTenPrimes;

    for (int i = 2; i < list->getSize(); i++)
    {
        if(list->getVal(i))
        {
            sum+=i;
        }
    } 
    std::cout << duration.count() << " " << list->getNumPrimes() << " " << sum << std::endl;
    for(int i = list->getSize(); i > 2; i--)
    {
        if(count == 10)
            break;
        if(list->getVal(i))
        {
            topTenPrimes.insert(topTenPrimes.begin(), i);
            count++;
        }      
    }   

    for(int i = 0; i < 10; i++)
        std::cout << topTenPrimes[i] << " ";
    
    std::cout << std::endl;
    return 0;
}