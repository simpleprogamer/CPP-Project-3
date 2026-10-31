#include <iostream>
#include <map>
#include <latch>
#include <string>
#include <vector>
#include <syncstream>
#include <thread>
#include <chrono>
#include <memory>
#include <functional>
struct Task
{
    char set;
    int i;
};

std::map <char, std::unique_ptr<std::latch>> L;

const std::map<char, std::string> deps =
{
    {'a', ""},  {'b', ""},  {'c', ""},
    {'e', "a"}, {'g', "a"}, {'d', "bc"},
    {'i', "e"}, {'j', "e"}, {'h', "dg"},
    {'k', "i"}, {'f', "hj"}
};

void f(char x, int i) 
{
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	std::osyncstream(std::cout) << "Task " << x << i << " completed." << std::endl;
}

void worker(const std::vector<Task>& plan)
{
    for (const auto& [x, i] : plan)
    {
        for (char d : deps.at(x)) L[d]->wait();
        f(x, i);
        L[x]->count_down();
    }
}


int main()
{
    const int nt = 4;
	const std::map<char, int> N = { {'a', 4}, {'b', 8}, {'c', 6}, {'d', 6}, {'e', 7}, {'f', 7}, {'g', 6}, {'h', 7}, {'i', 5}, {'j', 9}, {'k', 3} };
	for (const auto& [k, n] : N) L[k] = std::make_unique<std::latch>(n);
    const std::vector<std::vector<Task>> plans =
    {
         {{'a',1},{'e',1},{'e',5},{'b',1},{'b',5},{'c',1},{'c',5},{'d',1},{'d',5},
         {'g',6},{'h',1},{'h',5},{'i',2},{'j',4},{'j',8},{'f',1},{'f',5}},
         {{'a',2},{'e',2},{'e',6},{'b',2},{'b',6},{'c',2},{'c',6},{'d',2},{'d',6},
         {'j',1},{'h',2},{'h',6},{'i',3},{'j',5},{'j',9},{'f',2},{'f',6}},
         {{'a',3},{'e',3},{'e',7},{'b',3},{'b',7},{'c',3},{'g',2},{'d',3},{'g',4},
         {'j',2},{'h',3},{'h',7},{'i',4},{'j',6},{'k',1},{'f',3},{'f',7}},
         {{'a',4},{'e',4},{'g',1},{'b',4},{'b',8},{'c',4},{'g',3},{'d',4},{'g',5},
         {'j',3},{'h',4},{'i',1},{'i',5},{'j',7},{'k',2},{'f',4},{'k',3}},
    };
	std::osyncstream(std::cout) << "Starting tasks..." << std::endl;
	auto start = std::chrono::steady_clock::now();
    {
        std::vector<std::jthread> threads;
        for (int i = 0; i < nt; ++i) threads.emplace_back(worker, std::cref(plans[i]));
    }
	double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    std::osyncstream(std::cout) << "Elapsed time: " << elapsed << " seconds." << std::endl;
    return 0;
}

