/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <chrono>
#include <experimental/coroutine>
#include <system/Dispatcher.h>
#include <system/Awaiter.h>

using milliseconds = std::chrono::milliseconds;

class Dispatcher;

struct DelayAwaiter : public Awaiter<void>
{
    Dispatcher* dispatcher;
    milliseconds length;

    DelayAwaiter() = delete;
    DelayAwaiter(milliseconds length);

    DelayAwaiter(const DelayAwaiter&) = delete;
    DelayAwaiter& operator=(const DelayAwaiter&) = delete;

    DelayAwaiter(DelayAwaiter&&) = default;
    DelayAwaiter& operator=(DelayAwaiter&&) = default;

    bool ready() override;
    void suspend(std::experimental::coroutine_handle<> handle) override;
    void resume() override;

    static void Callback(void* handleAddr, intmax_t num);
};
