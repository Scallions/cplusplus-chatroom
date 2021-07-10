#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;
class server{
public:
	server(int port, string ip); // 构造函数
	~server();
	void run();
	static void RecvMsg(int conn);

private:
	int server_port; // 服务器端口后
	int server_sockfd; // listen fd
	string server_ip; // 服务器 ip
	vector<int> sock_arr; // 连接fd数组

};

#endif