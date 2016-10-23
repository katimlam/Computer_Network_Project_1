//
//  HttpRequest.h
//  hhh
//
//  Created by wenhui kuang on 10/15/16.
//  Copyright (c) 2016 wenhui kuang. All rights reserved.
//

#ifndef __hhh__HttpRequest__
#define __hhh__HttpRequest__

#include <stdio.h>
#include "HttpMessage.h"

class HttpRequest : public HttpMessage
{
private:
    HttpMethod m_method;
    URL m_url;
public:
    HttpRequest(HttpMethod method, URL url);
    HttpRequest();
    void comsume(string line);
    bool decodeFirstLine(string firstLine);
    string encode();
    URL getUrl(){return m_url;};
    void setMothod(HttpMethod method){m_method = method;};
    HttpMethod getMethod(){return m_method;};
    string getHost(){return m_url.host;};
    string getpath(){return m_url.path;};
};

HttpRequest::HttpRequest(HttpMethod method, URL url)
{
    m_method = method;
    m_url = url;
}
HttpRequest::HttpRequest()
{}
void HttpRequest::comsume(string line)
{
    size_t pos = line.find(" ");
    m_method = line.substr(0,pos);
    string temp = line.substr(pos+1);
    pos = temp.find(" ");
    m_url.path = temp.substr(0,pos);
    temp = temp.substr(pos+1);
    pos = temp.find("\r");
    setVersion(temp.substr(0,pos));
    temp = temp.substr(pos+2);
    pos = temp.find(":");
    m_url.host = temp.substr(0,pos+1);
}

string HttpRequest::encode()
{
    string request = getMethod() + " " + m_url.path + " " + getVersion() + "\r\n";
    request += "Host: " + m_url.host + "\r\n\r\n";
    return request;
    
}

#endif /* defined(__hhh__HttpRequest__) */
