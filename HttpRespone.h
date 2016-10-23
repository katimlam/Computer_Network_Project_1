//
//  HttpRespone.h
//  hhh
//
//  Created by wenhui kuang on 10/15/16.
//  Copyright (c) 2016 wenhui kuang. All rights reserved.
//

#ifndef __hhh__HttpRespone__
#define __hhh__HttpRespone__

#include <stdio.h>
#include "HttpMessage.h"

class HttpResponse : public HttpMessage
{
private:
    HttpMethod m_method;
    string contentLength;
    //string contentType;
    string status;
public:
    HttpResponse();
    void comsume(string line);
    bool decodeFirstLine(string firstLine);
    string encode();
    void setMothod(HttpMethod method){m_method = method;};
    HttpMethod getMethod(){return m_method;};
    string getStatus(){return status;};
    void setStatus(string m_status){status = m_status;};
    
};

HttpResponse::HttpResponse()
{
    contentLength = "0";
}

void HttpResponse::comsume(string line)
{
    string temp;
    size_t pos = line.find(" ");
    setVersion(line.substr(0,pos));
    temp = line.substr(pos+1);
    pos = temp.find(" ");
    status = temp.substr(0,pos);
}

string HttpResponse::encode()
{
    string response = getVersion() + " ";
    if (status == "200") {
        response += getStatus() + " OK\r\n";
    }
    else if (status == "400"){
        response += getStatus() + " Bad Request\r\n";
    }
    else if (status == "404"){
        response += getStatus() + " Not Found\r\n";
    }
    else if (status == "505"){
        response += getStatus() + " HTTP version not supported\r\n";
    }
    response += "Connection: close\r\n";
    if (contentLength != "0") {
        response += "Content-Length:" + contentLength + "\r\n" + "Content-Type: html";
    }
    else
        response += "Content-Type: html\r\n";
    return response;
}


#endif /* defined(__hhh__HttpRespone__) */
