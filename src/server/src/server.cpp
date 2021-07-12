#include "server.h"

vector<bool> server::sock_arr(100000, false);

server::server(int port, string ip): server_port(port), server_ip(ip) {
}

server::~server() {
	// 关闭打开的连接
	for(int i=0; i<sock_arr.size(); ++i){
		if(sock_arr[i]){
			close(i);
		}
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
		sock_arr[conn] = true;
		// 创建交互线程
		thread t(server::RecvMsg, conn);
		t.detach(); // 不用 join join会阻塞主线程
	}
}

// 子线程函数 不用添加 static
void server::RecvMsg(int conn) {
	char buffer[1000]; // 接受缓冲
	while(1){
		memset(buffer, 0, sizeof(buffer)); // 重置0 bzero一样
		int len = recv(conn, buffer, sizeof(buffer), 0);
		if(strcmp(buffer, "exit") == 0 || len<=0) {
			close(conn);
			sock_arr[conn] = false;
			break;
		}
		cout << "收到套接字: " << conn << " 信息: " << buffer << endl;
		string str(buffer);
		HandleRequest(conn, str);
		// // 回复
		// string ans = "收到";
		// int ret = send(conn, ans.c_str(), ans.length(), 0);
		// if(ret<=0){
		// 	close(conn);
		// 	sock_arr[conn] = false;
		// 	break;
		// }
	}
}

void server::HandleRequest(int conn, string str) {
	char buffer[1000];
	string name, pass;
	bool login_flag = false;
	string login_name;

	MYSQL *con = mysql_init(NULL);
	if(con == NULL) {
		cout << "Error init mysql: " << mysql_error(con) << endl;
	}
	if(!(mysql_real_connect(con, "10.133.1.91", "root", "gongli", "ChatProject", 0, NULL, CLIENT_MULTI_STATEMENTS))){
		cout << "Error connecting to database: " << mysql_error(con) << endl;
		return;
	}else{
		cout << "Connect db success" << endl;
	}

	if(str.find("name:") != str.npos) {
		// 处理注册 
		// TODO: 防止sql注入
		int p1 = str.find("name:"), p2 = str.find("pass:");
		name = str.substr(p1+5, p2-5);
		pass = str.substr(p2+5, str.length()-p2-4);
		string search = "INSERT INTO USER VALUE (\"";
		search += name;
		search += "\",\"";
		search += pass;
		search += "\");";
		cout << "sql语句: " << search << endl << endl;
		if(mysql_query(con, search.c_str())){
			cout << "Query failed: " << mysql_error(con) << endl;
		} else {
			cout << "Query success" << endl;
		}
	}else if(str.find("login") != str.npos) {
		int p1 = str.find("login"), p2 = str.find("pass:");
		name = str.substr(p1+5, p2-5);
		pass = str.substr(p2+5, str.length()-p2-4);
		string search = "SELECT * FROM USER WHERE NAME=\"";
		search += name;
		search += "\";";
		cout <<"sql语句: " << search << endl << endl;
		auto search_res = mysql_query(con, search.c_str());
		auto result = mysql_store_result(con);
		int col = mysql_num_fields(result);
		int row = mysql_num_rows(result);
		if(search_res == 0 && row!=0){
			cout << "查询成功" << endl;
			auto info = mysql_fetch_row(result);
			cout << "查询到用户名: " << info[0] << "密码: " << info[1] << endl;
			if(info[1] == pass) {
				cout << "登录密码正确" << endl << endl;
				string str1 = "ok";
				login_flag = true;
				login_name = name;
				send(conn, str1.c_str(), str1.length()+1, 0);
			}else{
				cout << "登录密码错误" << endl << endl;
				char str1[100] = "wrong";
				send(conn, str1, strlen(str1), 0);
			}
		}else{
			cout << "查询失败" << endl << endl;
			char str1[100] = "wrong";
			send(conn, str1, strlen(str1), 0);
		}
	}

	mysql_close(con);
}