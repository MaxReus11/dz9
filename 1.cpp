#define BOOST_DATE_TIME_NO_LIB
#include<iostream>
#include<string>
#include <future>
#include<chrono>
#include<map>
#define BOOST_DATE_TIME_NO_LIB
#include<iostream>
#include<string>
#include <future>
#include<chrono>
#include<map>
#include <fstream>
#include <thread>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/containers/string.hpp>

using namespace boost::interprocess;
using char_allocator = allocator<char, managed_shared_memory::segment_manager>;
using shared_string = basic_string<char, std::char_traits<char>, char_allocator>;
using pair_allocator = allocator < std::pair< int, shared_string>, managed_shared_memory::segment_manager >;
using shared_map = boost::unordered_map< int, shared_string, boost::hash<int>, std::equal_to<int>, pair_allocator>;



void add(managed_shared_memory shared_memory, shared_map* map_, int* ID, interprocess_mutex* mutex_, interprocess_condition* condition_, std::atomic<bool>* flag, std::atomic<bool>* end)
{
    auto line = shared_memory.find_or_construct<shared_string>("line")(shared_memory.get_segment_manager());
    do {
        getline(std::cin, *line);
        std::scoped_lock lock(*mutex_);
        map_->emplace(*ID, *line);
        *(ID)++;
        *flag = true;
        condition_->notify_all();
    }     while (!std::cin);
    *end = true;
}
void out(shared_map* map_, int* ID, interprocess_mutex* mutex_, interprocess_condition* condition_, std::atomic<bool>* flag, std::atomic<bool>* end)
{
    while (!end->load()) {
        std::unique_lock lock(*mutex_);
        condition_->wait(lock);
        if (!flag->load()) {
            std::cout << (map_->find(*(ID)-1))->second << std::endl;
        }
        *flag = false;
    }
}

int main() {
    
    std::ifstream fin("count.txt");
    int count;
    fin >> count;
    const std::string shared_memory_name = "managed_shared_memory";
    if (!count) {
        shared_memory_object::remove(shared_memory_name.c_str());

    }
    managed_shared_memory shared_memory( open_or_create, shared_memory_name.c_str(), 4096);
    count++;
    std::ofstream fout("count.txt");
    fout << count;
    fout.close();
    auto mutex_ = shared_memory.find_or_construct<interprocess_mutex>("m")();
    auto condition_ = shared_memory.find_or_construct<interprocess_condition>("c")();
    auto map_ = shared_memory.find_or_construct<shared_map>("Users")(shared_memory.get_segment_manager());
    auto ID_ = shared_memory.find_or_construct<int>("ID")(0);
    for (auto i : *map_) {
        std::cout << i.second << std::endl;
    }
    std::atomic<bool> flag = false, end = false;
    std::future<void> th1 = std::async(add, shared_memory, map_, ID_, mutex_, condition_, &flag, &end);
    std::future<void> th2 = std::async(out, map_, ID_, mutex_, condition_, &flag, &end);
    using namespace std::chrono_literals;
    while (!end.load()) {
        std::this_thread::sleep_for(2ms);
    }
    th1.get();
    th2.get();
    system("pause");
    fin >> count;
    fin.close();
    if (count == 1) {
        std::cout << count;
        boost::interprocess::shared_memory_object::remove(shared_memory_name.c_str());
    }
    count--;
    fout.open("count.txt");
    fout << count;
    fin.close();
    fout.close();
}
