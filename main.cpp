#include "threadpool.h"
#include <thread>
#include <vector>
#include <future>
#include <iostream>
#include <sstream>
#include <atomic>


auto do_cpu_cost_task(int m) -> std::tuple<std::string,int>
{
  for(int i = 0; i < 10000; i++){
    for(int j = 0; j < 20000; j++){

    }
  }
  std::ostringstream myss;
  auto th_id = std::this_thread::get_id();
  myss << th_id;
  std::tuple<std::string, int> res = std::make_tuple(myss.str(), m*2);
  return res;
}

class A {
  public:
    A():num(0) {}
    void a(){
      ThreadPool pool;
      auto result = pool.addTask(&A::b, this);
      auto result2 = pool.addTask(&A::b, this);
    }
    void printNum() {
      std::cout << num << '\n';
    }
  private:
    void b(){ std::cout << "A:b()" << ':';num++; }
    std::atomic<int> num;
};


int main()
{
  ThreadPool myPool;
  std::vector<std::future<std::tuple<std::string,int>>> results;
  for(int i = 0; i < 100; i++){
    results.emplace_back(myPool.addTask(do_cpu_cost_task, i));
  }
  for(int i = 0; i < 100; i++){
    auto result = results[i].get();
    std::cout << "result[" << i << "]" << " is " << std::get<0>(result) << "," << std::get<1>(result) << '\n';
  }
  A a;
  a.a();
  a.printNum();
  return 0;
}
