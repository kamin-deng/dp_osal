/*
 * Copyright (c) 2024 kamin.deng
 * Email: kamin.deng@gmail.com
 * Created on 2024/8/22.
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

#ifndef OSAL_CHRONO_H
#define OSAL_CHRONO_H

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

#include "interface_chrono.h"

namespace osal {

class OSALChrono : public IChrono {
public:
    using TimePoint = uint32_t;  // 使用内核计时器的滴答计数作为时间点
    using Duration = double;     // 使用秒作为时间间隔单位

    static OSALChrono &getInstance() {
        static OSALChrono instance;
        return instance;
    }

    // 获取当前时间点
    TimePoint now() const override {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        return static_cast<TimePoint>(duration.count());
    }

    // 计算两个时间点之间的时间间隔
    Duration elapsed(const TimePoint &start, const TimePoint &end) const override {
        return static_cast<Duration>(end - start) / 1000.0;  // 毫秒转换为秒
    }

    // 将时间点转换为time_t类型
    std::time_t to_time_t(const TimePoint &timePoint) const override {
        return static_cast<std::time_t>(timePoint / 1000);  // 毫秒转换为秒
    }

    // 将time_t类型转换为时间点
    TimePoint from_time_t(std::time_t time) const override {
        return static_cast<TimePoint>(time * 1000);  // 秒转换为毫秒
    }

    // 将时间点转换为字符串
    std::string to_string(const TimePoint &timePoint) const override {
        std::time_t time = to_time_t(timePoint);
        std::tm *tm = std::localtime(&time);
        std::ostringstream oss;
        oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

private:
    virtual ~OSALChrono() {}

    OSALChrono() {}
};

}  // namespace osal

#endif  // OSAL_CHRONO_H
