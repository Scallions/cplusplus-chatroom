#ifndef SERVER_H
#define SERVER_H

#include "global.h"
class server{
public:
	server(int port, string ip); // 构造函数
	~server();
	void run();
	static void RecvMsg(int conn);
	static void HandleRequest(int conn, string str); // 和连接交互

private:
	int server_port; // 服务器端口后
	int server_sockfd; // listen fd
	string server_ip; // 服务器 ip
	static vector<bool> sock_arr; // 连接fd数组

};

#endif