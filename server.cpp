#include "server.h"

server::server(int port, string ip): server_port(port), server_ip(ip) {
}

server::~server() {
	// 关闭打开的连接
	for(auto conn: sock_arr){
		close(conn);
	}
	close(server_sockfd);
}

void server::run() {
	// socket
	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// addr_in
	sockaddr_in server_sockaddr;
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(server_port);
	server_sockaddr.sin_addr.s_addr = inet_addr(server_ip.c_str());

	// bind
	if(::bind(server_sockfd, (sockaddr*)&server_sockaddr, sizeof(server_sockaddr)) == -1 ) { // 和std::bind 冲突 需要调用全局的bind
		perror("bind");
		exit(1);
	}

	// listen
	if(listen(server_sockfd,20) == -1){
		perror("listen");
		exit(1);
	}

	// client_addr
	sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	
	while(1) {
		int conn = accept(server_sockfd, (sockaddr*)&client_addr, &len);
		if(conn<0){
			perror("accept");
			exit(1);
		}
		cout << "文件描述符为: " << conn << " 的客户连接成功" << endl;
		sock_arr.push_back(conn);
		// 创建交互线程
		thread t(server::RecvMsg, conn);
		t.detach(); // 不用 join join会阻塞主线程
	}
}

// 子线程函数 不用添加 static
void server::RecvMsg(int conn) {
	char buffer[1000];
	while(1){
		memset(buffer, 0, sizeof(buffer));
		int len = recv(conn, buffer, sizeof(buffer), 0);
		if(strcmp(buffer, "exit") == 0 || len<=0) break;
		cout << "收到套接字: " << conn << " 信息: " << buffer << endl;
	}
}
