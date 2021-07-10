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
using namespace std;


int main() {
	// 定义fd
	int client_fd = socket(AF_INET, SOCK_STREAM, 0);

	// 定义服务器地址
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8023);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// connect
	if(connect(client_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0){ // 错误返回-1
		perror("connect");
		exit(1);
	}
	cout << "连接服务器成功" << endl;

	// 传输信息
	char sendbuff[100];
	char recvbuff[100];
	while(1){
		memset(sendbuff, 0, sizeof(sendbuff));
		cin >> sendbuff; // 从终端读取输入
		send(client_fd, sendbuff, strlen(sendbuff), 0);
		if(strcmp(sendbuff, "exit") == 0){
			break;
		}
	}
	close(client_fd);

	return 0;
}