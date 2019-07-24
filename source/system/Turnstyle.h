/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <vector>
#include <deque>
#include <mutex>
#include <atomic>
#include <experimental/coroutine>
#include <system/Dispatcher.h>
#include <system/Console.h>
#include <system/Spinlock.h>

class Turnstyle
{
    struct Guest {
        Dispatcher* dispatcher;
        std::experimental::coroutine_handle<> coroutine;
    };

    Spinlock lock;
    std::deque<Guest> lineup;
    size_t tokens = 0;
public:
    friend bool await_ready(Turnstyle& ts);
    friend void await_suspend(Turnstyle& ts, std::experimental::coroutine_handle<> handle);
    friend void await_resume(Turnstyle& ts);

    Turnstyle(){}

    void PermitOne()
    {
        std::lock_guard<Spinlock> lk(lock);

        if (lineup.empty())
        {
            ++tokens;
        }
        else
        {
            auto guest = lineup.front();
            lineup.pop_front();

            guest.dispatcher->InvokeAsync(
                [](auto p, auto n) { std::experimental::coroutine_handle<>::from_address(p).resume(); },
                guest.coroutine.address()
            );
        }
    }

    void PermitAll()
    {
        std::lock_guard<Spinlock> lk(lock);

        for (auto& guest : lineup)
        {
            guest.dispatcher->InvokeAsync(
                [](auto p, auto n) { std::experimental::coroutine_handle<>::from_address(p).resume(); },
                guest.coroutine.address()
            );
        }

        lineup.clear();
        tokens = 0;
    }

    friend struct TurnstyleAwaiter;
};

inline bool await_ready(Turnstyle& ts)
{
    // always return false to avoid monopolizing the dispatcher
    return false;
}

inline void await_suspend(Turnstyle& ts, std::experimental::coroutine_handle<> handle)
{
    std::lock_guard<Spinlock> lk(ts.lock);

    if (ts.tokens == 0)
    {
        ts.lineup.push_back(Turnstyle::Guest{ &Dispatcher::current(), handle });
    }
    else
    {
        --ts.tokens;

        Dispatcher::current().InvokeAsync(
            [](auto p, auto n) { std::experimental::coroutine_handle<>::from_address(p).resume(); },
            handle.address()
        );
    }
}

inline void await_resume(Turnstyle& ts)
{
    
}
