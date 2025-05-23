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

#include "osal_chrono.h"
#include "osal_system.h"
#include "osal_thread.h"
#include "test_framework.h"

using namespace osal;

TEST_CASE(TestOSALThreadStart) {
#if (TestOSALThreadStartEnabled)
    OSALThread thread;
    std::atomic<int> taskExecuted(1);

    thread.start(
        "TestThread",
        [&](void *) {
            taskExecuted = 2;
            OSALSystem::getInstance().sleep_ms(10000);
            taskExecuted = 3;
        },
        nullptr, 0, 1024);

    auto timestamp_now = OSALChrono::getInstance().now();
    OSALSystem::getInstance().sleep_ms(100);
    OSAL_ASSERT_EQ(taskExecuted, 2);
    thread.stop();
    auto interval = OSALChrono::getInstance().now() - timestamp_now;
    OSAL_ASSERT_EQ(taskExecuted, 2);
    OSAL_ASSERT_TRUE(interval > 50 && interval < 500);

    // 尝试再次启动同一个线程，应该失败
//    OSAL_ASSERT_TRUE(0 != thread.start("TestThread", [&](void*) {}));
#endif
    return 0;  // 表示测试通过
}

TEST_CASE(TestOSALThreadStop) {
#if (TestOSALThreadStopEnabled)
    OSALThread thread;
    std::atomic<bool> taskExecuted(false);

    thread.start("TestThread", [&](void *) { taskExecuted = true; }, nullptr, 0, 1024);

    thread.join();
    OSAL_ASSERT_TRUE(taskExecuted);

    // 尝试再次启动同一个线程，应该失败
//    OSAL_ASSERT_TRUE(0 != thread.start("TestThread", [&](void*) {}));
#endif
    return 0;  // 表示测试通过
}

TEST_CASE(TestOSALThreadJoin) {
#if (TestOSALThreadJoinEnabled)
    OSALThread thread;
    std::atomic<bool> taskExecuted(false);

    thread.start("TestThread", [&](void *) { taskExecuted = true; }, nullptr, 0, 1024);

    thread.join();
    OSAL_ASSERT_TRUE(taskExecuted);

    // 尝试再次join同一个线程，应该安全
    thread.join();
#endif
    return 0;  // 表示测试通过
}

TEST_CASE(TestOSALThreadDetach) {
#if (TestOSALThreadDetachEnabled)
    OSALThread thread;
    std::atomic<bool> taskExecuted(false);

    thread.start(
        "TestThread",
        [&](void *) {
            OSALSystem::getInstance().sleep_ms(50);
            taskExecuted = true;
        },
        nullptr, 0, 1024);

    thread.detach();
    OSALSystem::getInstance().sleep_ms(100);
    OSAL_ASSERT_TRUE(taskExecuted);

    // 尝试再次detach同一个线程，应该安全
    thread.detach();
#endif
    return 0;  // 表示测试通过
}

TEST_CASE(TestOSALThreadIsRunning) {
#if (TestOSALThreadIsRunningEnabled)
    OSALThread thread;
    std::atomic<bool> taskExecuted(false);

    thread.start(
        "TestThread",
        [&](void *) {
            OSALSystem::getInstance().sleep_ms(100);
            taskExecuted = true;
        },
        nullptr, 0, 1024);

    OSAL_ASSERT_TRUE(thread.isRunning());
    thread.join();
    OSAL_ASSERT_FALSE(thread.isRunning());

    // 尝试在线程结束后检查状态
    OSAL_ASSERT_FALSE(thread.isRunning());
#endif
    return 0;  // 表示测试通过
}

TEST_CASE(TestOSALThreadSetAndGetPriority) {
#if (TestOSALThreadSetAndGetPriorityEnabled)
    OSALThread thread;
    std::atomic<bool> taskExecuted(false);

    thread.start("TestThread", [&](void *) { taskExecuted = true; }, nullptr, 0, 1024);

    // 设置线程优先级
    int priority = 10;
    thread.setPriority(priority);

    // 获取线程优先级
    int retrievedPriority = thread.getPriority();

    OSAL_ASSERT_EQ(priority, retrievedPriority);

    priority = 5;
    // 尝试在线程结束后设置和获取优先级
    thread.setPriority(priority);
    OSAL_ASSERT_EQ(priority, thread.getPriority());

    thread.join();

    OSAL_ASSERT_TRUE(taskExecuted);

//    priority = 2;
//    // 尝试在线程结束后设置和获取优先级
//    thread.setPriority(priority);
//    OSAL_ASSERT_EQ(priority, thread.getPriority());
#endif
    return 0;  // 表示测试通过
}

TEST_CASE(TestOSALThreadSuspendAndResume) {
#if (TestOSALThreadSuspendAndResumeEnabled)
    OSALThread thread;
    std::atomic<bool> taskExecuted(false);

    std::function<void(void *)> taskFunction = [&](void *) {
        OSALSystem::getInstance().sleep_ms(100);
        taskExecuted = true;
    };

    thread.start("TestThread", taskFunction, nullptr, 0, 1024);

    // 暂停线程
    thread.suspend();
    OSALSystem::getInstance().sleep_ms(200);
    OSAL_ASSERT_FALSE(taskExecuted.load());

    // 恢复线程
    thread.resume();
    thread.join();
    OSAL_ASSERT_TRUE(taskExecuted.load());

    // 尝试在线程结束后暂停和恢复
    thread.suspend();
    thread.resume();
#endif
    return 0;  // 表示测试通过
}