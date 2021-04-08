#include <iostream>
#include <string>
//#include <mutex>
#include <thread>
#include <future>
#include <chrono>
#include <fstream>

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/containers/string.hpp>

using namespace boost::interprocess;
using allocator_str = allocator < char,
    managed_shared_memory::segment_manager >;
using string = basic_string <char, std::char_traits<char>, allocator_str>;
using allocator_map = allocator < std::string,
    managed_shared_memory::segment_manager >;
using b_map = boost::interprocess::map <int, std::string, allocator_map>;
void add(b_map* map_, int* ID, interprocess_mutex* mutex_, interprocess_condition* condition_, std::atomic<bool>* flag, std::atomic<bool>* end)
{
    std::string in;
    do {
        getline(std::cin, in);
        std::scoped_lock lock(*mutex_);
        map_->emplace(*ID,in);
        *ID++;
        *flag = true;
        condition_->notify_all();
    }     while (!std::cin);
    *end = true;
}
void out(b_map* map_, int* ID, interprocess_mutex* mutex_, interprocess_condition* condition_, std::atomic<bool>* flag, std::atomic<bool>* end)
{
    while (!end->load()) {
        std::unique_lock lock(*mutex_);
        condition_->wait(lock);
        if (!flag->load()) {
            std::cout << map_->find(*ID-1)->second << std::endl;
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
    managed_shared_memory shared_memory( open_or_create, shared_memory_name.c_str(), 1024);
    count++;
    std::ofstream fout("count.txt");
    fout << count;
    fout.close();
    auto mutex_ = shared_memory.find_or_construct<interprocess_mutex>("m")();
    auto condition_ = shared_memory.find_or_construct<interprocess_condition>("c")();
    auto map_ = shared_memory.find_or_construct<b_map>("map")(shared_memory.get_segment_manager());

    }
