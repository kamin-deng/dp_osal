/*
 * Copyright (c) 2024 kamin.deng
 * Email: kamin.deng@gmail.com
 * Created on 2024/8/23.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "osal_thread_pool.h"

#include <functional>

#include "osal_thread.h"

namespace osal {

OSALThreadPool::OSALThreadPool()
    : isstarted_(false), suspended_(false), priority_(0), activeThreads_(0), maxThreads_(0), minThreads_(0) {}

OSALThreadPool::~OSALThreadPool() { stop(); }

void OSALThreadPool::start(uint32_t numThreads, int priority, int stack_size) {
    stop();  // 停止任何现有的线程池
    isstarted_ = true;
    minThreads_ = numThreads;
    maxThreads_ = numThreads;
    priority_ = priority;
    stack_size_ = stack_size;
    for (uint32_t i = 0; i < numThreads; ++i) {
        OSALThreadPool::OSALAddTread();
    }
    OSAL_LOGD("Thread pool started with %u threads\n", numThreads);
}

bool OSALThreadPool::OSALAddTread() {
    auto thread = std::make_unique<OSALThread>();
    if (thread == nullptr) {
        OSAL_LOGE("Failed to create thread\n");
        return false;
    } else {
        thread->start("ThreadPool", threadEntry, this, priority_, stack_size_);
        threads_.push_back(std::move(thread));
        return true;
    }
}

bool OSALThreadPool::OSALDelTread() {
    bool result = false;
    for (auto &thread : threads_) {
        if (!thread->isRunning()) {
            thread->stop();
            threads_.erase(std::remove(threads_.begin(), threads_.end(), thread), threads_.end());
            result = true;
            break;
        }
    }
    return result;
}

void OSALThreadPool::stop() {
    isstarted_ = false;
    condition_.notify_all();
    for (std::shared_ptr<OSALThread> thread : threads_) {
        thread->stop();
    }
    threads_.clear();
    OSAL_LOGD("Thread pool stopped\n");
}

int OSALThreadPool::suspend() {
    suspended_ = true;
    OSAL_LOGD("Thread pool suspended\n");
    return 0;
}

int OSALThreadPool::resume() {
    suspended_ = false;
    condition_.notify_all();
    OSAL_LOGD("Thread pool resumed\n");
    return 0;
}

bool OSALThreadPool::isStarted() const { return isstarted_; }

bool OSALThreadPool::isSuspended() const { return suspended_; }

void OSALThreadPool::submit(std::function<void(void *)> taskFunction, void *taskArgument, int priority) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        taskQueue_.emplace(Task{taskFunction, taskArgument, priority});
    }
    condition_.notify_one();
    // 如果当前仍有任务堆积, 且已有线程池跑满，且未达到最大线程值
    if (activeThreads_ == std::size(threads_) && activeThreads_ < maxThreads_) {
        OSALAddTread();
    }
    OSAL_LOGD("Task submitted\n");
}

void OSALThreadPool::setPriority(int priority) {
    priority_ = priority;
    for (std::shared_ptr<OSALThread> thread : threads_) {
        thread->setPriority(priority);
        //            pthread_join(thread, nullptr);
    }
    OSAL_LOGD("Thread pool priority set to %d\n", priority);
}

int OSALThreadPool::getPriority() const { return priority_; }

size_t OSALThreadPool::getTaskQueueSize() {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return taskQueue_.size();
}

uint32_t OSALThreadPool::getActiveThreadCount() const { return activeThreads_; }

bool OSALThreadPool::cancelTask(std::function<void(void *)> &taskFunction) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    std::queue<Task> newQueue;
    bool found = false;
    auto targetPtr = taskFunction.template target<void (*)(void *)>();
    while (!taskQueue_.empty()) {
        Task task = taskQueue_.front();
        taskQueue_.pop();
        auto taskPtr = task.function.template target<void (*)(void *)>();
        // (!taskPtr && !targetPtr) 判断是为了处理两个空的 std::function 对象相等的情况。
        if ((taskPtr && targetPtr && *taskPtr == *targetPtr) || (!taskPtr && !targetPtr)) {
            found = true;
        } else {
            newQueue.push(task);
        }
    }
    taskQueue_ = std::move(newQueue);
    OSAL_LOGD("Task %s\n", found ? "cancelled" : "not found");
    return found;
}

void OSALThreadPool::setTaskFailureCallback(std::function<void(void *)> callback) {
    taskFailureCallback_ = callback;
    OSAL_LOGD("Task failure callback set\n");
}

void OSALThreadPool::setMaxThreads(uint32_t maxThreads) {
    maxThreads_ = maxThreads;
    OSAL_LOGD("Max threads set to %u\n", maxThreads);
}

uint32_t OSALThreadPool::getMaxThreads() const { return maxThreads_; }

void OSALThreadPool::setMinThreads(uint32_t minThreads) {
    minThreads_ = minThreads;
    OSAL_LOGD("Min threads set to %u\n", minThreads);
}

uint32_t OSALThreadPool::getMinThreads() const { return minThreads_; }

void OSALThreadPool::threadEntry(void *arg) {
    OSALThreadPool *pool = static_cast<OSALThreadPool *>(arg);
    pool->threadLoop();
}

void OSALThreadPool::threadLoop() {
    while (isstarted_) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            condition_.wait(lock, [this] { return !taskQueue_.empty() || !isstarted_; });
            if (!isstarted_) break;
            if (suspended_) continue;
            task = taskQueue_.front();
            taskQueue_.pop();
        }
        if (task.function != nullptr) {
            ++activeThreads_;
            task.function(task.argument);
            --activeThreads_;
        } else {
            if (taskFailureCallback_ != nullptr) {
                taskFailureCallback_(task.argument);
            }
        }
    }
}

}  // namespace osal