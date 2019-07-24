/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <exception>
#include <stdexcept>
#include <string>
#include <climits>

class socket_error : public std::exception
{
    int _error;
    std::string _message;
public:
    socket_error(const char* message, int error)
        : _error(error)
    {
        constexpr auto digits = (std::numeric_limits<int>::digits10 + 2);
        _message.reserve(sizeof("error: ") + strlen(message) + sizeof(" ()") + digits);
        _message += "error: ";
        _message += message;
        _message += " (";
        _message += std::to_string(error);
        _message += ") ";
    }

    int error() const {
        return _error;
    }

    virtual char const* what() const override {
        return _message.c_str();
    }
};
