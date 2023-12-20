
#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <functional>
#include <condition_variable>


template <typename T>

class safe_queue {
private:
    std::queue<T> tasks;
    std::mutex mutex;
    std::condition_variable cond_var;
public:
    void push(const T& task) {
        std::unique_lock<std::mutex> lock(mutex);
        tasks.push(task);
        lock.unlock();
        cond_var.notify_one();
    }
     T pop() {
        std::unique_lock<std::mutex> lock(mutex);
        cond_var.wait(lock, [this] {return !tasks.empty(); });
      
         T task = std::move( tasks.front());
        tasks.pop();
        return task;
    }
    bool empty() const {
        return tasks.empty();
    }
};

class thread_pool {
private:
    std::mutex mutex;
    std::vector<std::thread> threads;
    safe_queue<std::function<void()>> task_queue;
    std::atomic<bool> stop_flag;

public:
    thread_pool(size_t num_threads = std::thread::hardware_concurrency()) : stop_flag(false) {
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([this] { work(); });
        }
    }
    ~thread_pool() {
        stop_flag.store(true);
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    void submit(std::function<void()> task) {
        task_queue.push(task);
    }
private: 
        void work() {
            while (!stop_flag.load() ) {
                auto task = std::move(task_queue.pop());
                if (!task ) {
                    return;
                }
                task();
            }
        }
 };

void test_func_1() {
    std::cout << "test function 1 executable. Thread # "<< std::this_thread::get_id() << std::endl; 
}
void test_func_2() {
    std::cout << "test function 2 executable. Thread # " << std::this_thread::get_id() << std::endl;
}
void test_func_3() {
    std::cout << "test function 3 executable. Thread # " << std::this_thread::get_id() << std::endl;
}

int main()
{
    thread_pool pool;

    int test = 4;
    for (int i = 0; i < test; ++i) {
        pool.submit(test_func_1);
        pool.submit(test_func_2);
        //pool.submit(test_func_3);
        std::this_thread::sleep_for(std::chrono::seconds(1));

    }
    std::cout << "The end!";// из цикла выходим, но программа не завершилась, не могу понять причину;
                            // Есть версия, что work лезет в pop за задачей, но так и не выходит;
    return 0;
}
