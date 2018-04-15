/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <algorithm>
#include <regex>
#include <limits>
#include <unordered_map>
#include <optional>
#include "format.h"

enum class HttpMethod
{
    Connect,
    Delete,
    Get,
    Head,
    Options,
    Post,
    Put,
    Trace
};

enum class HttpStatus
{
    NotSet,
    Continue,
    SwitchingProtocols,
    OK,
    Created,
    Accepted,
    NonAuthoritativeInformation,
    NoContent,
    ResetContent,
    PartialContent,
    MultipleChoices,
    MovedPermanently,
    Found,
    SeeOther,
    NotModified,
    UseProxy,
    TemporaryRedirect,
    BadRequest,
    Unauthorized,
    PaymentRequired,
    Forbidden,
    NotFound,
    MethodNotAllowed,
    NotAcceptable,
    ProxyAuthenticationRequired,
    RequestTimeOut,
    Conflict,
    Gone,
    LengthRequired,
    PreconditionFailed,
    RequestEntityTooLarge,
    RequestURITooLarge,
    UnsupportedMediaType,
    RequestedRangeNotSatisfiable,
    ExpectationFailed,
    InternalServerError,
    NotImplemented,
    BadGateway,
    ServiceUnavailable,
    GatewayTimeOut,
    HTTPVersionNotSupported,
};

namespace Http
{
    typedef std::pair<std::optional<size_t>, std::optional<size_t>> ContentRange;

    std::vector<ContentRange> ParseRange(const std::string &field);
    std::string DecodeURL(const std::string& encoded);
    bool ParseRequestLine(const std::string& requestLine, HttpMethod& method, std::string& url, std::string& vers);
    bool ParseStatusLine(const std::string& statusLine, std::string& version, HttpStatus& code, std::string& reason);
    std::pair<std::string, std::string> ParseHeaderField(const std::string& line);
    std::vector<std::string> Split(const std::string &str, const std::string &delimeters);
}

class HttpRequest
{
public:
    HttpMethod method = HttpMethod::Get;
    std::string uri = "/";
    std::string version = "1.1";
    std::unordered_map<std::string, std::string> fields;
    std::vector<char> content;

    bool Parse(const std::vector<char>& request);
    bool Parse(const char *pRequest, size_t length);
    void Serialize(std::vector<char>& buffer);
};

class HttpResponse
{
public:
    std::string version = "1.1";
    HttpStatus status = HttpStatus::NotSet;
    std::string reason = "Not Set";
    std::unordered_map<std::string, std::string> fields;
    std::vector<char> content;

    bool Parse(const char *pResponse, size_t length);
    void Serialize(std::vector<char>& buffer);

    static HttpResponse Create(HttpStatus status, bool keepAlive = true);
};
