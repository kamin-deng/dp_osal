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

#include <atomic>

#include "osal_condition_variable.h"
#include "osal_mutex.h"
#include "osal_system.h"
#include "osal_thread.h"
#include "test_framework.h"

using namespace osal;

TEST_CASE(TestOSALConditionVariableWaitAndNotifyOne) {
#if (TestOSALConditionVariableWaitAndNotifyOneEnabled)
    osal::OSALMutex mutex;
    osal::OSALConditionVariable condVar;

    OSAL_ASSERT_TRUE(mutex.lock());

    // Start a thread to notify the condition variable after a delay
    OSALThread notifyThread;  // Assuming a factory function to create IThread instances
    notifyThread.start(
        "NotifyThread",
        [](void *arg) {
            OSALSystem::getInstance().sleep_ms(200);  // Simulate a delay
            static_cast<osal::OSALConditionVariable *>(arg)->notifyOne();
        },
        &condVar, 0, 1024);

    // Test waiting on the condition variable
    condVar.wait(mutex);
    OSAL_ASSERT_TRUE(mutex.unlock());

    notifyThread.join();  // Ensure the notify thread completes

#endif
    return 0;  // Indicates the test passed
}

TEST_CASE(TestOSALConditionVariableWaitForTimeout) {
#if (TestOSALConditionVariableWaitForTimeoutEnabled)
    osal::OSALMutex mutex;
    osal::OSALConditionVariable condVar;

    OSAL_ASSERT_TRUE(mutex.lock());

    // Start a thread to notify the condition variable after a longer delay
    OSALThread timeoutThread;
    timeoutThread.start(
        "TimeoutThread",
        [](void *arg) {
            OSALSystem::getInstance().sleep_ms(1000);  // Simulate a delay longer than the timeout
            static_cast<osal::OSALConditionVariable *>(arg)->notifyOne();
        },
        &condVar, 0, 1024);

    // Should return false as the wait should time out before notify
    OSAL_ASSERT_FALSE(condVar.waitFor(mutex, 500));
    OSAL_ASSERT_TRUE(mutex.unlock());

    timeoutThread.join();  // Ensure the timeout thread completes

#endif
    return 0;  // Indicates the test passed
}

TEST_CASE(TestOSALConditionVariableNotifyAll) {
#if (TestOSALConditionVariableNotifyAllEnabled)
    osal::OSALMutex mutex;
    osal::OSALConditionVariable condVar;
    std::atomic<int> taskExecutedCount(0);

    // Start multiple threads to wait on the condition variable
    auto worker = [&](void *arg) {
        (void)arg;
        mutex.lock();
        condVar.wait(mutex);  // Mutex must be accessible in this scope
        mutex.unlock();
        taskExecutedCount++;
    };

    OSALThread thread1("test_thread1", worker, nullptr, 0, 1024);
    OSALThread thread2("test_thread2", worker, nullptr, 0, 1024);
    OSALThread thread3("test_thread3", worker, nullptr, 0, 1024);

    OSALSystem::getInstance().sleep_ms(100);  // Ensure all threads are waiting
    OSAL_ASSERT_EQ(taskExecutedCount.load(), 0);
    {
        mutex.lock();
        condVar.notifyAll();
        mutex.unlock();
    }

    thread1.join();
    thread2.join();
    thread3.join();
    OSAL_ASSERT_EQ(taskExecutedCount.load(), 3);

#endif
    return 0;  // Indicates the test passed
}

TEST_CASE(TestOSALConditionVariableWaitCount) {
#if (TestOSALConditionVariableWaitCountEnabled)
    OSALConditionVariable condVar;
    OSALMutex mutex;

    auto workerTask = [&](void *) {
        mutex.lock();
        condVar.wait(mutex);
        mutex.unlock();
    };

    OSALThread thread1, thread2;
    thread1.start("TestThread1", workerTask, nullptr, 0, 1024);
    thread2.start("TestThread2", workerTask, nullptr, 0, 1024);

    OSALSystem::getInstance().sleep_ms(100);  // 确保线程进入等待状态
    OSAL_ASSERT_EQ(condVar.getWaitCount(), 2);

    {
        mutex.lock();
        condVar.notifyAll();
        mutex.unlock();
    }

    thread1.join();
    thread2.join();
    OSAL_ASSERT_EQ(condVar.getWaitCount(), 0);
#endif
    return 0;  // 表示测试通过
}