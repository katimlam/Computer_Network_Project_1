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

#include <iostream>
#include <sstream>
#include <string>
#include <thread>
using namespace std;

#define BUF_SIZE 512;
#define DEBUG 1;

int
main(int argc, const char* argv[])
{
	/////////// Get command line argument: hostname, port, and file directory //////////
	char* hostname, port, file_dir;
	
	if(argc == 1){
		hostname = "localhost";
		port = "4000";
		file_dir = ".";
	}
	else if(argc == 4){
		hostname = argv[1];
		port = argv[2];
		file_dir = argv[3];
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
	if ((status = getaddrinfo(hostname, port, &hints, &res)) != 0) {
		std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
		return 1;
	}
	
	if(DEBUG){
		std::cout << "IP addresses for " << argv[1] << ": " << std::endl;
	}

	for(struct addrinfo* p = res; p != 0; p = p->ai_next) {
		// convert address to IPv4 address
		struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
		// convert the IP to a string and print it:
		char ipstr[INET_ADDRSTRLEN] = {'\0'};
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
	addr.sin_port = htons(atoi(port));     // short, network byte order
	addr.sin_addr.s_addr = inet_addr(ipstr);
	memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("bind");
		return 2;
	}
	
	//////////////////////////////////////////////////////////////////////////////////
	
	// set socket to listen status
	if (listen(sockfd, 1) == -1) {
		perror("listen");
		return 3;
	}

	// accept a new connection
	struct sockaddr_in clientAddr;
	socklen_t clientAddrSize = sizeof(clientAddr);
	int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

  if (clientSockfd == -1) {
    perror("accept");
    return 4;
  }

  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  std::cout << "Accept a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << std::endl;

  // read/write data from/into the connection
  bool isEnd = false;
  char buf[20] = {0};
  std::stringstream ss;

  while (!isEnd) {
    memset(buf, '\0', sizeof(buf));

    if (recv(clientSockfd, buf, 20, 0) == -1) {
      perror("recv");
      return 5;
    }

    ss << buf << std::endl;
    std::cout << buf << std::endl;


    if (send(clientSockfd, buf, 20, 0) == -1) {
      perror("send");
      return 6;
    }

    if (ss.str() == "close\n")
      break;

    ss.str("");
  }

  close(clientSockfd);

  return 0;
}

/*
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

void
countdown(int id, int count)
{
  while (count > 0) {
    cout << "thread " << id << " is still alive." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    count--;
  }
  cout << "thread " << id << " is about to terminate." << std::endl;
}

void
noJoin()
{
  for (int i = 0; i < 3; i++) {
    thread(countdown, i+1, (i+1)*5).detach();
    // thread t(countdown, i+1, (i+1)*5); // Wrong! thread is not safely destroyed.
  }
  std::this_thread::sleep_for(std::chrono::seconds(20));
  cout << "main thread is about to terminate." << std::endl;
}

void
join()
{
  thread t[3];
  for (int i = 0; i < 3; i++) {
    t[i] = thread(countdown, i+1, (i+1)*5);
  }

  for (int i = 0; i < 3; i++) { // must join, otherwise threads will not be released safely.
    t[i].join();
  }
  cout << "main thread is about to terminate." << std::endl;
}

int main(int argc, char *argv[])
{
  noJoin();
  // join();
  return 0;
}
*/