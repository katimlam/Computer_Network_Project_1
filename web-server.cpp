//
//  web-server.cpp
//  hhh
//
//  Created by wenhui kuang on 10/15/16.
//  Copyright (c) 2016 wenhui kuang. All rights reserved.
//


#include "HttpRequest.h"
#include "HttpRespone.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <fstream>
using namespace std;

#define DEBUG 0

void new_connection(int clientSockfd,string file_dir);

int
main(int argc, const char* argv[])
{
    /////////// Get command line argument: hostname, port, and file directory //////////
    string hostname, port, file_dir;
    
    if(argc == 1){
        hostname = "localhost";
        port = "4000";
        file_dir = ".";
    }
    else if(argc == 4){
        hostname = argv[1];
        port = argv[2];
        file_dir = argv[3];
        file_dir = file_dir.substr(1);
    }
    else{
        std::cerr << "Invalid Argument" << endl;
        exit(EXIT_FAILURE);
    }
    
    ///////////////////////////////////// Show IP //////////////////////////////////////
    struct addrinfo hints;
    struct addrinfo* res;
    
    // prepare hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    
    // get address
    int status = 0;
    if ((status = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &res)) != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
        return 1;
    }
    
    //std::cout << "IP addresses for " << argv[1] << ": " << std::endl;
    char ipstr[INET_ADDRSTRLEN] = {'\0'};
    for(struct addrinfo* p = res; p != 0; p = p->ai_next) {
        // convert address to IPv4 address
        struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
        // convert the IP to a string and print it:
        inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
        if(DEBUG){
            std::cout << "  " << ipstr << std::endl;
        }
        // std::cout << "  " << ipstr << ":" << ntohs(ipv4->sin_port) << std::endl;
    }
    
    freeaddrinfo(res); // free the linked list
    //////////////////////////////////////////////////////////////////////////////////
    
    //create a socket using TCP IP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // allow others to reuse the address
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }
    
    // bind address to socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(port.c_str()));     // short, network byte order
    addr.sin_addr.s_addr = inet_addr(ipstr);
    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
    
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        return 2;
    }
    if (listen(sockfd, 1) == -1) {
        perror("listen");
        return 3;
    }
    
    while(1){
        struct sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);
        
        if (clientSockfd == -1) {
            perror("accept");
            return 4;
        }
        char ipstr[INET_ADDRSTRLEN] = {'\0'};
        inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
        if(DEBUG){
            std::cout << "Accept a connection from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << std::endl;
        }
        
        thread(new_connection, clientSockfd,file_dir).detach();
    }
    
    return 0;
}

void new_connection(int clientSockfd,string file_dir){
    
    
    // read/write data from the connection
    //bool isEnd = false;
    char buf[1024] = {0};
    std::stringstream ss;
    
    memset(buf, '\0', sizeof(buf));
    
    while (1) {
        if (recv(clientSockfd, buf, sizeof(buf), 0) == -1) {
            perror("recv");
            exit(1);
        }
        ss << buf;
        memset(buf, '\0', sizeof(buf));
        string temp = ss.str();
        size_t pos = temp.find("\r\n\r\n");
        if (pos != string::npos) {
            break;
        }
    }
    
    if(DEBUG){
        std::cout << &ss << std::endl;
    }
    
    HttpRequest request;
    HttpResponse response;
    request.comsume(ss.str());
    size_t pos = ss.str().find("Host:");
    
    if(DEBUG){
        cout << request.getHost() << endl;
        cout << request.getMethod() << endl;
        cout << request.getpath() << endl;
        cout << request.getVersion() << endl;
    }
    if (request.getMethod() != "GET" || pos == string::npos) {
        response.setStatus("400");
        response.setVersion(request.getVersion());
        string to_client = response.encode();
        if(write(clientSockfd,to_client.c_str(),to_client.length()) < 0){
            cout << "Error writing to socket" <<"\n";
            close(clientSockfd);
            return;
        }
    }
//    else if (request.getVersion() != "HTTP/1.0") {
//        response.setStatus("505");
//        response.setVersion(request.getVersion());
//        string to_client = response.encode();
//        if(write(clientSockfd,to_client.c_str(),to_client.length()) < 0){
//            cout << "Error writing to socket" <<"\n";
//            close(clientSockfd);
//            return;
//        }
//    }
    else
    {
        response.setStatus("200");
        response.setVersion(request.getVersion());
        
        string filePath = file_dir + request.getpath();
        cout << "Receiving a file from: " << filePath << endl;
        //string to_client = response.encode();
        //open file
        char buffer1[1024*1024];
        long readBytes = 0;
        int fd = open(filePath.c_str(), O_RDONLY);
        struct stat st;
        stat(filePath.c_str(), &st);
        long long size = st.st_size;
        
        response.setLength(to_string(size));
        string to_client = response.encode();
        
        if (fd >= 0) {
            if(write(clientSockfd,to_client.c_str(),to_client.length()) < 0){
                cout << "Error writing to socket" <<"\n";
                close(clientSockfd);
                return;
            }
            while((readBytes = read(fd, buffer1, sizeof(buffer1))) != 0){
                if(write(clientSockfd, buffer1, readBytes) < 0){
                    cout << "Error writing to socket" <<"\n";
                    close(clientSockfd);
                    return;
                }
                memset(&buffer1, 0, sizeof(buffer1));
                
            }
            
        }
        //        int fileSize = 0;
        //        string line;
        //        ifstream myfile(filePath);
        //        if (myfile.is_open()) {
        //            cout << "haha";
        //            if(write(clientSockfd,to_client.c_str(),to_client.length()) < 0){
        //                cout << "Error writing to socket" <<"\n";
        //                close(clientSockfd);
        //                return;
        //            }
        //            while (getline(myfile,line)) {
        //                line += line;
        //                cout << "haha";
        //                fileSize++;
        //            }
        //
        //            if(write(clientSockfd,line.c_str(),fileSize) < 0){
        //                cout << "Error writing to socket" <<"\n";
        //            }
        //            myfile.close();
        //        }
        
        else
        {
            response.setStatus("404");
            cout << fd << endl;
            string to_client = response.encode();
            if(write(clientSockfd,to_client.c_str(),to_client.length()) < 0){
                cout << "Error writing to socket.." <<"\n";
                close(clientSockfd);
                return;
            }
            
        }
        
    }
    
    close(clientSockfd);
}