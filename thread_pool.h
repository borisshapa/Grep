#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <functional>
#include <vector>
#include <thread>
#include <condition_variable>
#include <queue>
#include <future>
#include <QString>
#include "grep_task.h"

struct thread_pool
{
    explicit thread_pool(size_t);

    ~thread_pool();

    void enqueue(std::shared_ptr<grep_task_t>);

    void wait_empty_queue();

private:
    std::vector<std::thread> threads;
    std::condition_variable event;
    std::mutex m;
    bool stopping = false;
    std::queue<std::shared_ptr<grep_task_t>> tasks_queue;

    void start(size_t num_threads);

    void stop();
};

#endif // THREAD_POOL_H
