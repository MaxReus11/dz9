#define BOOST_DATE_TIME_NO_LIB
#include<iostream>
#include<string>
#include <future>
#include<chrono>
#include<map>
#include<atomic>
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

class Messenger {
private:
    int* count;
    const std::string shared_memory_name = "managed_shared_memory";
    managed_shared_memory shared_memory;
    interprocess_mutex* mutex_;
    interprocess_condition* condition_;
    shared_map* map_;
    int* ID_;
    shared_string* line;
    std::atomic<bool> flag = false, end = false;
public:
    Messenger() {
        shared_memory = managed_shared_memory(open_or_create, shared_memory_name.c_str(), 4096);

        
        count = shared_memory.find_or_construct<int>("Count")(0);
        mutex_ = shared_memory.find_or_construct<interprocess_mutex>("m")();
        condition_ = shared_memory.find_or_construct<interprocess_condition>("c")();
        map_ = shared_memory.find_or_construct<shared_map>("Users")(shared_memory.get_segment_manager());
        ID_ = shared_memory.find_or_construct<int>("ID")(0);
        line = shared_memory.find_or_construct<shared_string>("line")(shared_memory.get_segment_manager());
        (*count)++;
       }
    void add()
    {
        do {
            getline(std::cin, *line);
            std::scoped_lock lock(*mutex_);
            map_->emplace(*ID_, *line);
            (*ID_)++;
            flag = true;
            condition_->notify_all();
        } while (*line != "0");
          end = true;
    }
    void out()
    {
        while (!end.load()) {
            std::unique_lock lock(*mutex_);
            condition_->wait(lock);
            if (!flag.load()) {
                std::cout << (map_->find((*ID_)-1))->second << std::endl;
            }
            flag = false;
        }
    }
    void start() {
        for (auto i : *map_) {
            std::cout << i.second << std::endl;
        }
        std::future<void> th1 = std::async(&Messenger::add, this);
        std::future<void> th2 = std::async(&Messenger::out, this);
        using namespace std::chrono_literals;
        while (!end.load()) {
            std::this_thread::sleep_for(2ms);
        }
        th1.get();
        th2.get();
        system("pause");

    }
    ~Messenger() {

       if (*count == 1) {
              boost::interprocess::shared_memory_object::remove(shared_memory_name.c_str());
          }
          count--;
      }
};

int main() {
    Messenger m;
    m.start();
    return EXIT_SUCCESS;
    
}
