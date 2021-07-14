#ifndef SERVER_H
#define SERVER_H

#include "global.h"
#include <unordered_map>
#include <mutex>
#include <tuple>
#include <set>
#include <mysql.h>
#include <hiredis/hiredis.h>
#include <sys/event.h>
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

	// 用户相关
	static unordered_map<string, int> name_sock_map;
	static mutex name_sock_mutex;

	// conn info
	static unordered_map<int, tuple<bool, string, string, int, int>> conn_map;
	static mutex conn_mutex;

	// 群相关
	static unordered_map<int, set<int>> group_map; // 群号和套接字
	static mutex group_mutex; // 群聊锁

	// db
	static MYSQL* con;

	// redis
	static redisContext *redis_target;

	// kqueue
	static void setnonblocking(int conn);
	static int kq;

};

#endif