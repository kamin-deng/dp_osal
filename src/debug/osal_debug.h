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

#ifndef __OSAL_DEBUG_H__
#define __OSAL_DEBUG_H__

#include <stdarg.h>
#include <stdio.h>

#include "osal.h"

namespace osal {

// 日志等级定义
enum LogLevel {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_VERBOSE
};

void setLogLevel(LogLevel level);

LogLevel getLogLevel();

void setIncludeFileFunctionLine(bool include);

bool getIncludeFileFunction();

// 宏定义
#define DEFINE_LOG_FUNCTION(logFuncName, prefix, level)                                           \
    void logFuncName(const char* file, const char* function, int line, const char* format, ...) { \
        if (getLogLevel() >= level) {                                                             \
            va_list args;                                                                         \
            va_start(args, format);                                                               \
            common_log(prefix, file, function, line, format, args);                               \
            va_end(args);                                                                         \
        }                                                                                         \
    }

#define DECLARE_LOG_FUNCTION(logFuncName) \
    void logFuncName(const char* file, const char* function, int line, const char* format, ...);

DECLARE_LOG_FUNCTION(OSAL_LOG_)

DECLARE_LOG_FUNCTION(OSAL_LOGA_)

DECLARE_LOG_FUNCTION(OSAL_LOGE_)

DECLARE_LOG_FUNCTION(OSAL_LOGW_)

DECLARE_LOG_FUNCTION(OSAL_LOGI_)

DECLARE_LOG_FUNCTION(OSAL_LOGD_)

DECLARE_LOG_FUNCTION(OSAL_LOGV_)

// 宏定义，用于简化日志调用
#define OSAL_LOG(format, ...) OSAL_LOG_(__FILE__, __FUNCTION__, __LINE__, format __VA_OPT__(, ) __VA_ARGS__)
#define OSAL_LOGA(format, ...) OSAL_LOGA_(__FILE__, __FUNCTION__, __LINE__, format __VA_OPT__(, ) __VA_ARGS__)
#define OSAL_LOGE(format, ...) OSAL_LOGE_(__FILE__, __FUNCTION__, __LINE__, format __VA_OPT__(, ) __VA_ARGS__)
#define OSAL_LOGW(format, ...) OSAL_LOGW_(__FILE__, __FUNCTION__, __LINE__, format __VA_OPT__(, ) __VA_ARGS__)
#define OSAL_LOGI(format, ...) OSAL_LOGI_(__FILE__, __FUNCTION__, __LINE__, format __VA_OPT__(, ) __VA_ARGS__)
#define OSAL_LOGD(format, ...) OSAL_LOGD_(__FILE__, __FUNCTION__, __LINE__, format __VA_OPT__(, ) __VA_ARGS__)
#define OSAL_LOGV(format, ...) OSAL_LOGV_(__FILE__, __FUNCTION__, __LINE__, format __VA_OPT__(, ) __VA_ARGS__)

}  // namespace osal

#endif  // __OSAL_DEBUG_H__