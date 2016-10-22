//
//  Header.h
//  hhh
//
//  Created by wenhui kuang on 10/15/16.
//  Copyright (c) 2016 wenhui kuang. All rights reserved.
//

#ifndef HttpMessage_h
#define HttpMessage_h

#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

typedef string HttpVersion;
typedef string HttpMethod;
typedef int HttpStatus;

struct URL{
    string port;
    string path;
    string host;
};
class HttpMessage
{
private:
    
    HttpVersion m_version;
    map<string,string> m_headers;
public:
    HttpMessage();
    HttpVersion getVersion(){return m_version;};
    string getHeader(string key);
    string getAllHeaders();
    void comsume(string line);
    void setVersion(HttpVersion httpVersion){m_version = httpVersion;};
    void getHeader(string key, string value){m_headers[key] = value;};
    void decodeHeaderLine(string &headerLines);
    void setHeaders(string key, string value);
    void combineHeaders();
};

HttpMessage::HttpMessage()
{
    m_version = "";
    m_headers.clear();
}

void HttpMessage::decodeHeaderLine(string &headerLines)
{
    
}

void HttpMessage::setHeaders(string key, string value)
{
    m_headers.insert(pair<string, string>(key,value));
}

void HttpMessage::combineHeaders()
{
    map<string, string>::iterator it;
    
}

#endif
