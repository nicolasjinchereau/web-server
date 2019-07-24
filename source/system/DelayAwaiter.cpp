/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include <system/DelayAwaiter.h>

DelayAwaiter::DelayAwaiter(milliseconds length)
{
    dispatcher = &Dispatcher::current();
    this->length = length;
}

bool DelayAwaiter::ready() {
    return length == milliseconds(0);
}

void DelayAwaiter::suspend(std::experimental::coroutine_handle<> handle)
{
    auto resumeTime = DispatchClock::now() + length;

    dispatcher->InvokeAsync(
        &DelayAwaiter::Callback,
        handle.address(),
        0,
        DispatchPriority::Normal,
        resumeTime
    );
}

void DelayAwaiter::resume()
{
}

void DelayAwaiter::Callback(void* handleAddr, intmax_t num)
{
    auto handle = std::experimental::coroutine_handle<>::from_address(handleAddr);
    handle.resume();
}
