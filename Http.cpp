/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include "Http.h"
using namespace std;

struct EnumClassHash
{
    template <typename T>
    auto operator()(T t) const {
        return static_cast<std::underlying_type<T>::type>(t);
    }
};

const unordered_map<HttpStatus, string, EnumClassHash> statusCodes =
{
    { HttpStatus::NotSet, "0" },
    { HttpStatus::Continue, "100" },
    { HttpStatus::SwitchingProtocols, "101" },
    { HttpStatus::OK, "200" },
    { HttpStatus::Created, "201" },
    { HttpStatus::Accepted, "202" },
    { HttpStatus::NonAuthoritativeInformation, "203" },
    { HttpStatus::NoContent, "204" },
    { HttpStatus::ResetContent, "205" },
    { HttpStatus::PartialContent, "206" },
    { HttpStatus::MultipleChoices, "300" },
    { HttpStatus::MovedPermanently, "301" },
    { HttpStatus::Found, "302" },
    { HttpStatus::SeeOther, "303" },
    { HttpStatus::NotModified, "304" },
    { HttpStatus::UseProxy, "305" },
    { HttpStatus::TemporaryRedirect, "307" },
    { HttpStatus::BadRequest, "400" },
    { HttpStatus::Unauthorized, "401" },
    { HttpStatus::PaymentRequired, "402" },
    { HttpStatus::Forbidden, "403" },
    { HttpStatus::NotFound, "404" },
    { HttpStatus::MethodNotAllowed, "405" },
    { HttpStatus::NotAcceptable, "406" },
    { HttpStatus::ProxyAuthenticationRequired, "407" },
    { HttpStatus::RequestTimeOut, "408" },
    { HttpStatus::Conflict, "409" },
    { HttpStatus::Gone, "410" },
    { HttpStatus::LengthRequired, "411" },
    { HttpStatus::PreconditionFailed, "412" },
    { HttpStatus::RequestEntityTooLarge, "413" },
    { HttpStatus::RequestURITooLarge, "414" },
    { HttpStatus::UnsupportedMediaType, "415" },
    { HttpStatus::RequestedRangeNotSatisfiable, "416" },
    { HttpStatus::ExpectationFailed, "417" },
    { HttpStatus::InternalServerError, "500" },
    { HttpStatus::NotImplemented, "501" },
    { HttpStatus::BadGateway, "502" },
    { HttpStatus::ServiceUnavailable, "503" },
    { HttpStatus::GatewayTimeOut, "504" },
    { HttpStatus::HTTPVersionNotSupported, "505" },
};

const unordered_map<string, HttpStatus> statusNames =
{
    { "0", HttpStatus::NotSet },
    { "100", HttpStatus::Continue },
    { "101", HttpStatus::SwitchingProtocols },
    { "200", HttpStatus::OK },
    { "201", HttpStatus::Created },
    { "202", HttpStatus::Accepted },
    { "203", HttpStatus::NonAuthoritativeInformation },
    { "204", HttpStatus::NoContent },
    { "205", HttpStatus::ResetContent },
    { "206", HttpStatus::PartialContent },
    { "300", HttpStatus::MultipleChoices },
    { "301", HttpStatus::MovedPermanently },
    { "302", HttpStatus::Found },
    { "303", HttpStatus::SeeOther },
    { "304", HttpStatus::NotModified },
    { "305", HttpStatus::UseProxy },
    { "307", HttpStatus::TemporaryRedirect },
    { "400", HttpStatus::BadRequest },
    { "401", HttpStatus::Unauthorized },
    { "402", HttpStatus::PaymentRequired },
    { "403", HttpStatus::Forbidden },
    { "404", HttpStatus::NotFound },
    { "405", HttpStatus::MethodNotAllowed },
    { "406", HttpStatus::NotAcceptable },
    { "407", HttpStatus::ProxyAuthenticationRequired },
    { "408", HttpStatus::RequestTimeOut },
    { "409", HttpStatus::Conflict },
    { "410", HttpStatus::Gone },
    { "411", HttpStatus::LengthRequired },
    { "412", HttpStatus::PreconditionFailed },
    { "413", HttpStatus::RequestEntityTooLarge },
    { "414", HttpStatus::RequestURITooLarge },
    { "415", HttpStatus::UnsupportedMediaType },
    { "416", HttpStatus::RequestedRangeNotSatisfiable },
    { "417", HttpStatus::ExpectationFailed },
    { "500", HttpStatus::InternalServerError },
    { "501", HttpStatus::NotImplemented },
    { "502", HttpStatus::BadGateway },
    { "503", HttpStatus::ServiceUnavailable },
    { "504", HttpStatus::GatewayTimeOut },
    { "505", HttpStatus::HTTPVersionNotSupported },
};

const unordered_map<HttpStatus, string, EnumClassHash> statusReasons =
{
    { HttpStatus::NotSet, "Not Set"},
    { HttpStatus::Continue, "Continue"},
    { HttpStatus::SwitchingProtocols, "Switching Protocols"},
    { HttpStatus::OK, "OK"},
    { HttpStatus::Created, "Created"},
    { HttpStatus::Accepted, "Accepted"},
    { HttpStatus::NonAuthoritativeInformation, "Non Authoritative Information"},
    { HttpStatus::NoContent, "No Content"},
    { HttpStatus::ResetContent, "Reset Content"},
    { HttpStatus::PartialContent, "Partial Content"},
    { HttpStatus::MultipleChoices, "Multiple Choices"},
    { HttpStatus::MovedPermanently, "Moved Permanently"},
    { HttpStatus::Found, "Found"},
    { HttpStatus::SeeOther, "See Other"},
    { HttpStatus::NotModified, "Not Modified"},
    { HttpStatus::UseProxy, "Use Proxy"},
    { HttpStatus::TemporaryRedirect, "Temporary Redirect"},
    { HttpStatus::BadRequest, "Bad Request"},
    { HttpStatus::Unauthorized, "Unauthorized"},
    { HttpStatus::PaymentRequired, "Payment Required"},
    { HttpStatus::Forbidden, "Forbidden"},
    { HttpStatus::NotFound, "Not Found"},
    { HttpStatus::MethodNotAllowed, "Method Not Allowed"},
    { HttpStatus::NotAcceptable, "Not Acceptable"},
    { HttpStatus::ProxyAuthenticationRequired, "Proxy Authentication Required"},
    { HttpStatus::RequestTimeOut, "Request TimeOut"},
    { HttpStatus::Conflict, "Conflict"},
    { HttpStatus::Gone, "Gone"},
    { HttpStatus::LengthRequired, "Length Required"},
    { HttpStatus::PreconditionFailed, "Precondition Failed"},
    { HttpStatus::RequestEntityTooLarge, "Request Entity Too Large"},
    { HttpStatus::RequestURITooLarge, "Request URI Too Large"},
    { HttpStatus::UnsupportedMediaType, "Unsupported Media Type"},
    { HttpStatus::RequestedRangeNotSatisfiable, "Requested Range Not Satisfiable"},
    { HttpStatus::ExpectationFailed, "Expectation Failed"},
    { HttpStatus::InternalServerError, "Internal Server Error"},
    { HttpStatus::NotImplemented, "Not Implemented"},
    { HttpStatus::BadGateway, "Bad Gateway"},
    { HttpStatus::ServiceUnavailable, "Service Unavailable"},
    { HttpStatus::GatewayTimeOut, "Gateway Time Out"},
    { HttpStatus::HTTPVersionNotSupported, "HTTP Version Not Supported"}
};

const unordered_map<HttpMethod, string, EnumClassHash> methodNames =
{
    { HttpMethod::Connect, "CONNECT"},
    { HttpMethod::Delete, "DELETE"},
    { HttpMethod::Get, "GET"},
    { HttpMethod::Head, "HEAD"},
    { HttpMethod::Options, "OPTIONS"},
    { HttpMethod::Post, "POST"},
    { HttpMethod::Put, "PUT"},
    { HttpMethod::Trace, "TRACE"},
};

const unordered_map<string, HttpMethod> methods =
{
    { "CONNECT", HttpMethod::Connect},
    { "DELETE", HttpMethod::Delete},
    { "GET", HttpMethod::Get},
    { "HEAD", HttpMethod::Head },
    { "OPTIONS", HttpMethod::Options },
    { "POST", HttpMethod::Post },
    { "PUT", HttpMethod::Put },
    { "TRACE", HttpMethod::Trace },
};

namespace Http
{
    char FromHex(char ch) {
        return isdigit(ch) ? ch - '0' : (::tolower(ch)) - 'a' + 10;
    }

    string DecodeURL(const string& encoded)
    {
        string ret;
        ret.reserve(encoded.size() - count(encoded.begin(), encoded.end(), '%'));

        for(auto it = encoded.begin(); it != encoded.end(); ++it)
        {
            auto ch = *it;

            if(ch == '%')
            {
                if(it + 2 < encoded.end()) {
                    ch = FromHex(it[1]) << 4 | FromHex(it[2]);
                    it += 2;
                }
            }
            else if(ch == '+')
            {
                ch = ' ';
            }

            ret.push_back(ch);
        }

        return ret;
    }

    bool ParseRequestLine(const string& requestLine, HttpMethod& method, string& url, string& vers)
    {
        regex reg("(CONNECT|DELETE|GET|HEAD|OPTIONS|POST|PUT|TRACE) (.+) HTTP/(.+)");
        smatch match;

        if(!regex_search(requestLine, match, reg) || match.size() != 4)
            return false;

        auto it = methods.find(match[1]);
        if(it == methods.end())
            return false;

        method = it->second;
        url = match[2];
        vers = match[3];
        return true;
    }

    bool ParseStatusLine(const string& statusLine, string& version, HttpStatus& code, string& reason)
    {
        regex reg("HTTP/([\\d\\.]+) (\\d{3}) (.+)");
        smatch match;

        if(!regex_search(statusLine, match, reg) || match.size() != 4)
            return false;

        auto it = statusNames.find(match[2]);
        if(it == statusNames.end())
            return false;

        version = match[1];
        code = it->second;
        reason = match[3];
        return true;
    }

    pair<string, string> ParseHeaderField(const string& line)
    {
        pair<string, string> parts;

        smatch match;
        regex reg("\\s*(.+)\\s*:\\s*(.+)\\s*");

        if(regex_search(line, match, reg)) {
            parts.first = match[1];
            parts.second = match[2];
        }

        return parts;
    }

    vector<ContentRange> ParseRange(const string &field)
    {
        regex reg("bytes\\s*=\\s*(\\d*)\\s*-\\s*(\\d*)\\s*(?:\\s*,\\s*(\\d*)\\s*-\\s*(\\d*))*");
        smatch match;

        vector<ContentRange> ret;

        if(regex_search(field, match, reg))
        {
            for(int i = 1; i < (int)match.size() - 1; i += 2)
            {
                string match1 = match[i + 0];
                string match2 = match[i + 1];
                auto start = !match1.empty() ? optional<size_t>{ atol(match1.c_str()) } : nullopt;
                auto end = !match2.empty() ? optional<size_t>{ atol(match2.c_str()) } : nullopt;
                ret.push_back(make_pair(start, end));
            }
        }

        return ret;
    }

    vector<string> Split(const string &str, const string &delimeters)
    {
        vector<string> ret;

        size_t start = 0;
        size_t found = str.find_first_of(delimeters, start);

        while(found != string::npos)
        {
            size_t len = found - start;
            if(len > 0)
                ret.push_back(str.substr(start, len));

            start = str.find_first_not_of(delimeters, found);
            found = str.find_first_of(delimeters, start);
        }

        if(start < str.length())
            ret.push_back(str.substr(start));

        return ret;
    }
}

using namespace Http;

bool HttpRequest::Parse(const vector<char>& request) {
    return Parse(request.data(), request.size());
}

bool HttpRequest::Parse(const char *pRequest, size_t length)
{
    auto end = strstr(pRequest, "\r\n\r\n");
    if(end == nullptr)
        return false;

    size_t headerLength = end - pRequest;
    string header(pRequest, headerLength);

    vector<string> lines = Split(header, "\r\n");
    if(lines.empty())
        return false;

    if(!ParseRequestLine(lines[0], this->method, this->uri, this->version))
        return false;

    for(size_t i = 1; i < lines.size(); ++i)
    {
        auto parts = ParseHeaderField(lines[i]);

        if(parts.first.empty() || parts.second.empty())
            return false;

        fields.insert(move(parts));
    }

    const char *contentStart = pRequest + headerLength + 4;
    const char *contentEnd = contentStart + (length - headerLength - 4);
    content.assign(contentStart, contentEnd);

    return true;
}

void HttpRequest::Serialize(vector<char>& buffer)
{
    stringstream request;

    request << std::noskipws;

    request << methodNames.find(method)->second << " " << uri << " " << "HTTP/" << version << "\r\n";

    for(auto& field : fields)
        request << field.first << ": " << field.second << "\r\n";

    request << "\r\n";

    size_t sz = (size_t)request.tellp();

    buffer.clear();
    buffer.reserve(sz + content.size());
    buffer.resize(sz);

    request.seekg(0, ios::beg);
    request.read(buffer.data(), sz);

    if(!content.empty())
        buffer.insert(buffer.end(), content.begin(), content.end());
}

bool HttpResponse::Parse(const char *pResponse, size_t length)
{
    size_t headerLength = strstr(pResponse, "\r\n\r\n") - pResponse;
    string header(pResponse, headerLength);

    vector<string> lines = Split(header, "\r\n");
    if(lines.empty())
        return false;

    string reason;
    ParseStatusLine(lines[0], version, status, reason);

    for(size_t i = 1; i < lines.size(); ++i)
    {
        auto parts = ParseHeaderField(lines[i]);

        if(parts.first.empty() || parts.second.empty())
            return false;

        fields.insert(move(parts));
    }

    const char *contentStart = pResponse + headerLength + 4;
    const char *contentEnd = contentStart + (length - headerLength - 4);
    content.assign(contentStart, contentEnd);

    return true;
}

void HttpResponse::Serialize(vector<char>& buffer)
{
    stringstream response;
    response << std::noskipws;
    response << "HTTP/" << version << " " << statusCodes.at(status) << " " << statusReasons.at(status) << "\r\n";

    for(auto& field : fields)
        response << field.first << ": " << field.second << "\r\n";

    response << "\r\n";

    size_t headerSize = (size_t)response.tellp();
    buffer.clear();
    buffer.reserve(headerSize + content.size());
    buffer.resize(headerSize);

    response.seekg(0, ios::beg);
    response.read(buffer.data(), headerSize);

    if(!content.empty())
        buffer.insert(buffer.end(), content.begin(), content.end());
}

HttpResponse HttpResponse::Create(HttpStatus status, bool keepAlive)
{
    string code = statusCodes.at(status);
    string reason = statusReasons.at(status);
    string page = format("<html><h1 style=\"text-align: center\">%: %</h1></html>", code, reason);

    HttpResponse resp;
    resp.status = status;
    resp.fields["Connection"] = keepAlive ? "keep-alive" : "close";
    resp.fields["Content-Encoding"] = "identity";
    resp.fields["Content-Type"] = "text/html; charset=utf-8";
    resp.fields["Content-Length"] = to_string(page.size());
    resp.content.assign(page.begin(), page.end());
    return resp;
}
