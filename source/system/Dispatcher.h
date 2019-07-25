/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <string>
#include <experimental/coroutine>
#include <atomic>
#include <queue>
#include <iostream>
#include <functional>
#include <cassert>
#include <chrono>
#include <system/PriorityQueue.h>
#include <system/Console.h>

using DispatchClock = std::chrono::steady_clock;
using DispatchTime = std::chrono::steady_clock::time_point;

enum class DispatchPriority : int
{
    Low,
    Normal,
    High
};

struct DispatchAction
{
    typedef std::chrono::microseconds Duration;

    void(*fun)(void* ptr, intmax_t num) = nullptr;
    void* ptr = nullptr;
    intmax_t num = 0;
    DispatchPriority pri = DispatchPriority::Normal;
    DispatchTime tim = DispatchTime(std::chrono::microseconds(0));
    void(*fin)(void* ptr, intmax_t num) = nullptr;

    bool operator==(const DispatchAction& right) const { return pri == right.pri; }
    bool operator!=(const DispatchAction& right) const { return pri != right.pri; }
    bool operator<(const DispatchAction& right) const { return pri < right.pri; }
    bool operator<=(const DispatchAction& right) { return pri <= right.pri; }
    bool operator>(const DispatchAction& right) { return pri > right.pri; }
    bool operator>=(const DispatchAction& right) { return pri >= right.pri; }
};

class Dispatcher
{
    mutable std::mutex mut;
    mutable std::condition_variable cv;
    std::atomic<bool> run = false;
    PriorityQueue<DispatchAction*> requests;

    void InvokeAsync(DispatchAction* req)
    {
        std::unique_lock<std::mutex> lk(mut);
        requests.push(req);

        if (run && req->tim <= DispatchClock::now())
            Wake();
    }

    void InvokeFunction(DispatchAction* req)
    {
        try
        {
            if (req->fun)
                req->fun(req->ptr, req->num);
        }
        catch (std::exception& ex) {
            Console::WriteLine("Dispatcher: error invoking finalizer - ", ex.what());
        }
    }

    void InvokeFinalizer(DispatchAction* req)
    {
        try
        {
            if (req->fin)
                req->fin(req->ptr, req->num);
        }
        catch (std::exception& ex) {
            Console::WriteLine("Dispatcher: error invoking finalizer - ", ex.what());
        }
    }

public:
    
    static Dispatcher& current() {
        static thread_local Dispatcher disp;
        return disp;
    }

    Dispatcher()
    {
    }

    ~Dispatcher() {
        requests.unordered_enumerate([](DispatchAction* req) { delete req; });
        requests.clear();
    }

    DispatchAction* InvokeAsync(
        void(*function)(void* ptr, intmax_t num),
        void* ptr = nullptr,
        intmax_t num = 0,
        DispatchPriority pri = DispatchPriority::Normal,
        DispatchTime tim = DispatchTime(std::chrono::microseconds(0)),
        void(*fin)(void* ptr, intmax_t num) = nullptr
    )
    {
        auto req = new DispatchAction{ function, ptr, num, pri, tim, fin };
        InvokeAsync(req);
        return req;
    }

    void Remove(DispatchAction* req)
    {
        if (req)
        {
            if(requests.remove(req))
                InvokeFinalizer(req);
        }
    }

    void Run()
    {
        run = true;

        while (run)
        {
            auto req = WaitForRequest();

            if (run && req) {
                InvokeFunction(req.get());
                InvokeFinalizer(req.get());
            }
        }
    }

    void Quit()
    {
        std::unique_lock<std::mutex> lk(mut);
        run = false;
        Wake();
    }

private:
    void Wake() {
        cv.notify_one();
    }

    std::unique_ptr<DispatchAction> WaitForRequest()
    {
        std::unique_lock<std::mutex> lk(mut);
        
        if (!requests.empty() && requests.top()->tim > DispatchClock::now())
            cv.wait_until(lk, requests.top()->tim, [this]{ return !run || (!requests.empty() && requests.top()->tim <= DispatchClock::now()); });
        else
            cv.wait(lk, [this] { return !run || !requests.empty(); });
        
        std::unique_ptr<DispatchAction> req;

        if (run && !requests.empty()) {
            req.reset(requests.top());
            requests.pop();
        }

        return req;
    }
};
