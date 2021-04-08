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
    boost::interprocess::managed_shared_memory::segment_manager >;
using string = basic_string <char, std::char_traits<char>, allocator_str>;
using allocator_map = allocator < std::string,
    managed_shared_memory::segment_manager >;
using b_map = boost::interprocess::map <std::string,
   std::string, allocator_map>;
using mutex = interprocess_mutex;
using condition = interprocess_condition;
void add(b_map* map_, mutex* mutex_, condition* condition_, std::atomic<bool>* flag, std::atomic<bool>* end)
{
    std::string in;
    do {
        getline(std::cin, in);
        std::scoped_lock lock(*mutex_);
        map_->emplace(std::chrono::steady_clock::now(),in);
        *flag = true;
        condition_->notify_all();
    }     while (!std::cin);
    *end = true;
}


int main() {
    

    const std::string shared_memory_name = "managed_shared_memory";

    shared_memory_object::remove(shared_memory_name.c_str());

    managed_shared_memory shared_memory( open_or_create, shared_memory_name.c_str(), 1024);

    }
