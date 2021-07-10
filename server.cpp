#include <sys/types.h>
#include <sys/socket.h>
#include <cstdio>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <iostream>

using namespace std;

int main() {
	// 定义 socketfd
	int server_sockfd = socket(AF_INET, SOCK_STREAM, 0); // 需要定义ip协议，和传输层协议

	// 定义sockaddr_in
	sockaddr_in server_sockaddr;
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(8023);
	server_sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// bind
	// 需要判断是否绑定成功，可能会有其他程序占用端口
	if(bind(server_sockfd, (sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) == -1)
	{
		perror("bind");
		exit(1);
	};

	// listen
	if(listen(server_sockfd, 20) == -1){
		perror("listen");
		exit(1);
	}

	// accept
	// 定义客户端套接字
	sockaddr_in client_addr;
	socklen_t length = sizeof(client_addr);

	// 客户端fd
	int conn = accept(server_sockfd, (sockaddr*)&client_addr, &length);
	if(conn < 0) {
		perror("connect");
		exit(1);
	}
	cout << "客户端连接成功" << endl;

	// 接受缓冲区
	char buffer[1000];

	// 接受数据
	while(1) {
		memset(buffer, 0, sizeof(buffer));
		int len = recv(conn, buffer, sizeof(buffer), 0);
		if(strcmp(buffer, "exit") == 0 || len<=0) break;
		cout << "收到客户端信息: " << buffer << endl;
	}

	// 关闭连接
	close(conn);
	close(server_sockfd);
	return 0;
}