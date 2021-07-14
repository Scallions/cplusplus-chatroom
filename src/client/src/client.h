#ifndef CLIENT_H
#define CLIENT_H

#include "global.h"

class client{

public:
	client(int port, string ip);
	~client();
	void run();
	static void SendMsg(int conn); // 发送线程
	static void RecvMsg(int conn); // 接受线程
	void HandleClient(int conn);

private:
	int server_port;
	string server_ip;
	int sock; // conn fd
	static bool recv_flag;
};


#endif