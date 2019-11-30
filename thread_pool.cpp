#include "thread_pool.h"
#include <future>
#include <QDebug>

thread_pool::thread_pool(size_t num_threads) {
    start(num_threads);
}

thread_pool::~thread_pool() {
    stop();
}

void thread_pool::enqueue(std::shared_ptr<grep_task_t> task) {
    {
        std::unique_lock<std::mutex> lg(m);
        tasks_queue.emplace(task);
    }

    event.notify_one();
}

void thread_pool::start(size_t num_threads) {
    for (size_t i = 0; i < num_threads; i++) {
        threads.emplace_back([&] {
            while(true) {
                std::shared_ptr<grep_task_t> task;

                {
                    std::unique_lock<std::mutex> lg(m);
                    event.wait(lg, [=] {return stopping || !tasks_queue.empty(); });

                    if (stopping && tasks_queue.empty())
                        break;

                    task = tasks_queue.front();
                    tasks_queue.pop();
                }

                task->grep_file();
            }
        });
    }
}

void thread_pool::stop() {
    {
        std::unique_lock<std::mutex> lg(m);
        stopping = true;
    }

    event.notify_all();

    for (auto& thread : threads) {
        thread.join();
    }
}

void thread_pool::wait_empty_queue() {
    std::unique_lock<std::mutex> lg(m);
    event.wait(lg, [=] {return tasks_queue.empty(); });
}
