#include <thread>
#include <future>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

class ThreadPool{
  public:
    ThreadPool(ssize_t maxTz=std::thread::hardware_concurrency()):isStopped(false){
      for(int i = 0; i < maxTz; i++){
        workers.emplace_back(std::thread(&ThreadPool::thread_routine, this));
      }
    }
    ~ThreadPool(){
      {
        std::unique_lock<std::mutex> lck(task_mutex);
        isStopped = true;
      }
      task_cv.notify_all();
      for(auto &worker : workers){
        worker.join();
      }
    }
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    template<class F, class... Args>
       auto addTask(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );
        std::future<return_type> res = task->get_future();
        {
          std::unique_lock<std::mutex> lck(task_mutex);
          if(isStopped){
            throw std::runtime_error("could not dispatch task on stopped pool.");
          }
          taskQueue.emplace([task](){(*task)();});
        }
        task_cv.notify_one();
        return res;
      }
  private:
    void thread_routine();
    std::mutex task_mutex;
    std::condition_variable task_cv;
    ssize_t maxThreads;
    bool isStopped;
    std::vector<std::thread> workers;
    std::queue<std::function<void()> > taskQueue;

};


inline void ThreadPool::thread_routine(){
  for(;;){
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lck(task_mutex);
      task_cv.wait(lck, [this]{ return this->isStopped || !this->taskQueue.empty(); } );
      if(isStopped){
        return;
      }
      task = taskQueue.front();
      taskQueue.pop();
    }
    task();
  }
}
