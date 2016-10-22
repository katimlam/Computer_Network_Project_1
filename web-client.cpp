#include "HttpRequest.h"
#include "HttpRespone.h"
#include <iostream>
#include <string>

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <fstream>

using namespace std;

URL parseUrl(string url)
{
    string path, host, port;
    string http = "http://";
    URL temp_url;
    temp_url.host = "";
    temp_url.path = "";
    temp_url.port = "";
    
    string temp_string = url.substr(0,http.size());
    
    cout << temp_string << endl;
    
    if (temp_string == http) {
        temp_string = url.substr(http.size());
        // cout << temp_string << endl;
        size_t pos = temp_string.find(":");
        cout << pos << endl;
        if (pos == string::npos) {
            cout << "Incorret URL format." << endl;
            exit(0);
        }
        if (pos == 0) {
            cout << "Empty host name." << endl;
            exit(1);
        }
        temp_url.host = temp_string.substr(0,pos);
        temp_string = temp_string.substr(pos+1);
        pos = temp_string.find("/");
        if (pos == string::npos) {
            cout << "Incorret URL format." << endl;
            exit(0);
        }
        if (pos == 0) {
            port = "80";
        }
        temp_url.port = temp_string.substr(0,pos);
        temp_url.path = temp_string.substr(pos);
    }
    else
    {
        perror("Not a HTTP.");
        exit(1);
    }
    return temp_url;
}

string getIP(URL &url)
{
    struct addrinfo hints;
    struct addrinfo* res;
    
    // prepare hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    
    // get address
    int status = 0;
    if ((status = getaddrinfo(url.host.c_str(), url.port.c_str(), &hints, &res)) != 0) {
        cerr << "getaddrinfo: " << gai_strerror(status) << endl;
        exit(2);
    }
    
    char ipstr[INET_ADDRSTRLEN] = {'\0'};
    for(struct addrinfo* p = res; p != 0; p = p->ai_next) {
        // convert address to IPv4 address
        struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
        
        // convert the IP to a string and print it:
        inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
        // std::cout << "  " << ipstr << std::endl;
        // std::cout << "  " << ipstr << ":" << ntohs(ipv4->sin_port) << std::endl;
    }
    
    freeaddrinfo(res); // free the linked list
    
    cout << "IP addresses of " << url.host << ": " << ipstr << endl;
    return ipstr;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        cerr << "Not Enough Arguments...Pass in URL" << endl;
        exit(0);
    }
    
    URL *url = new URL[argc-1];
    
    for (int i = 0; i < argc - 1; i++) {
        url[i] = parseUrl(argv[i+1]);
        if (url[i].path == "/") {
            url[i].path = "/index.html";
        }
        cerr<<i<<"th URL path is:"<<url[i].path<<" URL host is:"<<url[i].host<<" URL port is:"<<url[i].port<<std::endl;
    }
    for (int i = 0; i < argc-1; i++) {
        string IP = getIP(url[i]);
        //======= establish connection
        
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(atoi(url[i].port.c_str()));     // short, network byte order
        
        std::cerr<<"sin_port works"<<std::endl;
        
        serverAddr.sin_addr.s_addr = inet_addr(IP.c_str());
        memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
        
        std::cerr<<"memset serveraddr works"<<std::endl;
        
        
        // connect to the server
        if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
            perror("connect");
            return 1;
        }
        
        cerr << "Connection succeed" << endl;
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
            perror("getsockname");
            return 1;
        }
        
        char ipstr[INET_ADDRSTRLEN] = {'\0'};
        inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
        std::cerr << "Set up a connection from: " << ipstr << ":" <<
        ntohs(clientAddr.sin_port) << std::endl;
        
        //set up the http request
        
        HttpRequest request("GET", url[i]);
        request.setVersion("HTTP/1.0");
        string EncodedRequest = request.encode();
        
        cerr << "Test request is: "<< endl <<EncodedRequest;
        
        // send data over socket
        if (send(sockfd, EncodedRequest.c_str(), strlen(EncodedRequest.c_str()), 0) == -1) {
            perror("Could not send to server.\n");
            close(sockfd);
            exit(1);
        }
        
        cout << "HTTP request sent, awaiting response... " << endl;
        
        char buffer[1024];
        char singleChar;
        memset(buffer,0,sizeof(buffer));
        string reply = "";
        long read = 0;
        while((recv(sockfd,&singleChar,sizeof(singleChar),0)) > 0){
            reply += singleChar;
            if (reply.find("\r\n\r\n") != string::npos) {
                break;
            }
            
        }
        cout << reply << endl;
        
        HttpResponse response;
        response.comsume(reply);
        string status = response.getStatus();
        string filename = url[i].path.substr(1);
        
        if (status == "200") {
            cout << "200 OK" << endl;
            cout << "Saving file to " << filename << endl;
            
        }
        else if (status == "400"){
            cout << "400 Bad request" << endl;
            close(sockfd);
            exit(1);
        }
        else if (status == "404"){
            cout << "404 Not Found" << endl;
            close(sockfd);
            exit(1);
        }
        else if (status == "505"){
            cout << "505 HTTP version not supported" << endl;
            close(sockfd);
            exit(1);
        }
        
        fstream file;
        file.open(filename,ios::out|ios::binary);
        while ((read = recv(sockfd, &buffer, sizeof(buffer), 0)) > 0) {
            file.write(buffer, read);
            file.flush();
            memset(buffer, 0, sizeof(buffer));
        }
        file.close();
        close(sockfd);
    }
    
    
}

