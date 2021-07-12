#include "client.h"


client::client(int port, string ip): server_port(port), server_ip(ip) {
}

client::~client() {
	close(sock);
}

void client::run() {
	// socket
	sock = socket(AF_INET, SOCK_STREAM, 0);

	// server addr 
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

	if(connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		perror("connect");
		exit(1);
	}
	cout << "连接服务器成功" << endl;

	HandleClient(sock);

	// 创建发送线程和接受线程
	// thread send_t(SendMsg, sock), recv_t(RecvMsg, sock);
	// send_t.join();
	// cout << "发送线程结束" << endl;
	// recv_t.join();
	// cout << "接收线程结束" << endl;
}

void client::HandleClient(int conn) {
	int choice; 
	string name, pass, pass1;
	bool login_flag = false; // 是否登录成功
	string login_name; // 记录用户名

	// 打印信息
    cout<<" ------------------\n";
    cout<<"|                  |\n";
    cout<<"| 请输入你要的选项:|\n";
    cout<<"|    0:退出        |\n";
    cout<<"|    1:登录        |\n";
    cout<<"|    2:注册        |\n";
    cout<<"|                  |\n";
    cout<<" ------------------ \n\n";

	// 处理登录相关
	while(1) {
		if(login_flag) {
			break;
		}
		cin >> choice;
		if(choice == 0) {
			break;
		}
		else if(choice == 2) {
			// 注册
			cout << "注册的用户名:";
			cin >> name;
			// int count = 0;
			while(1) {
				cout << "密码:";
				cin >> pass;
				cout << "确认密码:";
				cin >> pass1;
				if(pass == pass1) {
					break;
				}else{
					cout << "两次密码不一致!" << endl;
					// ++count;
					// if(count>2){
					// 	cout << "超过三次密码错误，注册失败" << endl;
					// 	break;
					// }
				}
			}
			name = "name:"+name;
			pass = "pass:"+pass;
			string str = name+pass;
			send(conn, str.c_str(), str.length(), 0);
			cout << "注册成功" << endl;
			cout << "\n继续输入你要的选项:";
		}else if(choice ==1 &&!login_flag) {
			while(1) {
				cout << "用户名:";
				cin >> name;
				cout << "密码:";
				cin >> pass;
				string str = "login" + name;
				str += "pass:";
				str += pass;
				send(sock, str.c_str(), str.length(), 0);
				char buffer[1000];
				memset(buffer, 0, sizeof(buffer));
				recv(sock, buffer, sizeof(buffer), 0);
				string recv_str(buffer);
				// 判断是否登录成功
				if(recv_str.substr(0,2) == "ok") {
					login_flag = true;
					login_name = name;
					cout << "登录成功\n" << endl;
					break;
				}else{
					cout << "用户名或者密码错误!\n" << endl;
				}
			}
		}
	}

	// 登录成功
	if(login_flag){
		system("clear"); // 清空终端
        cout<<"        欢迎回来,"<<login_name<<endl;
        cout<<" -------------------------------------------\n";
        cout<<"|                                           |\n";
        cout<<"|          请选择你要的选项：               |\n";
        cout<<"|              0:退出                       |\n";
        cout<<"|              1:发起单独聊天               |\n";
        cout<<"|              2:发起群聊                   |\n";
        cout<<"|                                           |\n";
        cout<<" ------------------------------------------- \n\n";
	}
}

// 发送线程
void client::SendMsg(int conn) {
	char sendbuff[100];
	while(1){
		memset(sendbuff, 0, sizeof(sendbuff));
		cin >> sendbuff;
		int ret = send(conn, sendbuff, strlen(sendbuff), 0);
		if(strcmp(sendbuff, "exit") == 0 || ret <=0){
			break;
		}
	}
}

// 接收线程
void client::RecvMsg(int conn) {
	char buffer[100];
	while(1) {
		memset(buffer, 0, sizeof(buffer));
		int len = recv(conn, buffer, sizeof(buffer), 0);
		if(len<=0){
			break;
		}
		cout << "接收服务器信息: " <<  buffer << endl;
	}
}