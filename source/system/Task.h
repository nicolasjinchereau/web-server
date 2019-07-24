/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <utility>
#include <type_traits>
#include <algorithm>
#include <memory>
#include <system/Awaiter.h>
#include <system/DelayAwaiter.h>

template<class T>
class Task
{
public:
    Task(Task&&) = default;
    Task& operator=(Task&&) = default;

	Task(const std::shared_ptr<Awaiter<T>>& awaiter)
        : awaiter(awaiter)
    {
    }
    
    Task(std::shared_ptr<Awaiter<T>>&& awaiter)
        : awaiter(std::move(awaiter))
    {
    }

	bool await_ready() {
        return awaiter->ready();
	}

	void await_suspend(std::experimental::coroutine_handle<> handle) {
        awaiter->suspend(handle);
	}

	T await_resume() {
        return awaiter->resume();
	}

public:
    static Task<void> Delay(milliseconds length) {
        return Task<void>(std::make_shared<DelayAwaiter>(length));
    }

private:
    std::shared_ptr<Awaiter<T>> awaiter;
};

template<class T>
class CoroutineAwaiter;

template<>
class CoroutineAwaiter<void> : public Awaiter<void>
{
public:
    Dispatcher* dispatcher = nullptr;
    std::experimental::coroutine_handle<> myHandle = nullptr;
    std::experimental::coroutine_handle<> parentHandle = nullptr;
    std::exception_ptr ex;

    CoroutineAwaiter(Dispatcher* dispatcher, std::experimental::coroutine_handle<> myHandle)
        : dispatcher(dispatcher), myHandle(myHandle)
    {
    }

    virtual bool ready() override {
        return false;
    }

    virtual void suspend(std::experimental::coroutine_handle<> handle) override {
        parentHandle = handle;
    }

    virtual void resume() override {
        auto ex_ptr = ex;
        if (ex_ptr)
            std::rethrow_exception(ex_ptr);
    }
};

template<class T>
class CoroutineAwaiter : public CoroutineAwaiter<void>
{
    T value;
public:
    virtual T resume() override {
        CoroutineAwaiter<void>::resume();
        return value;
    }
};

namespace std::experimental
{
    template<class T, class... Args>
    struct coroutine_traits<Task<T>, Args...>
    {
        struct promise_type
        {
            std::shared_ptr<CoroutineAwaiter<T>> awaiter;

            Task<T> get_return_object()
            {
                auto dispatcher = &Dispatcher::current();
                auto myHandle = coroutine_handle<>::from_address(coroutine_handle<promise_type>::from_promise(*this).address());
                awaiter = std::make_shared<CoroutineAwaiter<T>>(dispatcher, myHandle);
                return Task<T>(awaiter);
            }

            bool initial_suspend() const {
                return false;
            }

            bool final_suspend() const {
                return true;
            }

            template<class U>
            void return_value(U&& value)
            {
                awaiter->value = std::forward<U>(value);
                DispatchCompletion();
            }

            void set_exception(exception_ptr ex)
            {
                awaiter->ex = ex;
                DispatchCompletion();
            }

            void DispatchCompletion()
            {
                awaiter->dispatcher->InvokeAsync([](auto p, auto n) {
                    shared_ptr<CoroutineAwaiter<T>> awaiter = ((promise_type*)p)->awaiter;
                    awaiter->myHandle.destroy();
                    if (awaiter->parentHandle) awaiter->parentHandle.resume();
                }, this);
            }
        };
    };

    template<class... Args>
    struct coroutine_traits<Task<void>, Args...>
    {
        struct promise_type
        {
            std::shared_ptr<CoroutineAwaiter<void>> awaiter;

            Task<void> get_return_object()
            {
                auto dispatcher = &Dispatcher::current();
                auto myHandle = coroutine_handle<>::from_address(coroutine_handle<promise_type>::from_promise(*this).address());
                awaiter = std::make_shared<CoroutineAwaiter<void>>(dispatcher, myHandle);
                return Task<void>(awaiter);
            }

            bool initial_suspend() const {
                return false;
            }

            bool final_suspend() const {
                return true;
            }

            void return_void() {
                DispatchCompletion();
            }

            void set_exception(exception_ptr ex) {
                awaiter->ex = ex;
                DispatchCompletion();
            }

            void DispatchCompletion()
            {
                awaiter->dispatcher->InvokeAsync([](auto p, auto n) {
                    shared_ptr<CoroutineAwaiter<void>> awaiter = ((promise_type*)p)->awaiter;
                    awaiter->myHandle.destroy();
                    if (awaiter->parentHandle) awaiter->parentHandle.resume();
                }, this);
            }
        };
    };
}
