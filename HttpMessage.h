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
typedef string ByteBlob;

struct URL{
  string path;
  string host;
  string port;
};

class HttpRequest
{
private:
  
  HttpVersion m_version;
  map<string,string> m_headers;
public:
  bool deFirstLine(ByteBlob firstLine);
  HttpVersion getVersion(){return m_version;};
  string getHeader(string key);
  string getAllHeaders();
  void setVersion(HttpVersion httpVersion){m_version = httpVersion;};
  void setHeader(string key, string value);
  void decodeHeaderLine(ByteBlob headerLines);
}
