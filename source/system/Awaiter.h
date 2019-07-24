/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <cstdint>
#include <chrono>
#include <vector>
#include <exception>
#include <string>
#include <vector>
#include <experimental/coroutine>

template<class T>
class Awaiter
{
public:
    virtual ~Awaiter(){}
    virtual bool ready() = 0;
    virtual void suspend(std::experimental::coroutine_handle<> handle) = 0;
    virtual T resume() = 0;
};
