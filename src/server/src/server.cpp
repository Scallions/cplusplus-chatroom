#include "server.h"

vector<bool> server::sock_arr(100000, false);
unordered_map<string, int> server::name_sock_map;
mutex server::name_sock_mutex;
unordered_map<int, set<int>> server::group_map;
mutex server::group_mutex;
MYSQL *server::con = NULL;
redisContext *server::redis_target = NULL;

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
	mysql_close(con);
	redisFree(redis_target);
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

	// init db
	con = mysql_init(NULL);
	if(con == NULL) {
		cout << "Error init mysql: " << mysql_error(con) << endl;
	}
	if(!(mysql_real_connect(con, "10.133.1.91", "root", "gongli", "ChatProject", 0, NULL, CLIENT_MULTI_STATEMENTS))){
		cout << "Error connecting to database: " << mysql_error(con) << endl;
		return;
	}else{
		cout << "Connect db success" << endl;
	}

	// init redis
	redis_target = redisConnect("10.133.1.91", 6379);
	if(redis_target->err){
		redisFree(redis_target);
		cout << "连接redis失败" << endl;
	}else{
		cout << "连接redis成功" << endl;
	}
	// auth
	redisReply *r = (redisReply *)redisCommand(redis_target, "AUTH %s", "gongli");
	if(r->str){
		cout << "验证redis成功" << endl;
	}else{
		redisFree(redis_target);
		cout << "验证redis失败" << endl;
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
	// info 维持此次连接信息
	tuple<bool, string, string, int, int> info; // login_flag, login_name, target_name, target_conn, group
	get<0>(info) = false;
	get<3>(info) = -1;

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
		HandleRequest(conn, str, info);
	}
}

void server::HandleRequest(int conn, string str, tuple<bool, string, string, int, int> &info) {
	char buffer[1000];
	string name, pass;
	bool login_flag = get<0>(info);
	string login_name = get<1>(info);
	string target_name = get<2>(info);
	int target_conn = get<3>(info);
	int group_num = get<4>(info);

	// TODO: 处理逻辑提出去
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
				{
					lock_guard<mutex> lck(name_sock_mutex);
					name_sock_map[login_name] = conn;
				}

				// 设置session
				srand(time(NULL));
				for(int i=0; i<10; ++i) {
					int type = rand()%3;
					if(type==0){
						str1+='0'+rand()%9;
					}else if(type==1){
						str1+='a'+rand()%26;
					}else if(type==2){
						str1+='A'+rand()%26;
					}
				}
				// 存入redis
				string redisstr = "hset " + str1.substr(2) + " name " + login_name;
				redisReply *r = (redisReply *)redisCommand(redis_target, redisstr.c_str());
				// 设置过期时间
				redisstr = "expire " + str1.substr(2) + " 300";
				r = (redisReply *)redisCommand(redis_target, redisstr.c_str());
				cout << "用户: " << login_name << " 的sessionid为: " << str1.substr(2) << endl;

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
	}else if (str.find("target:") != str.npos){
		// 新建私聊
		int pos1 = str.find("from");
		string target = str.substr(7, pos1-7), from = str.substr(pos1+4);
		target_name = target;
		if(name_sock_map.find(target_name) == name_sock_map.end()){
			// 没找到
			cout<<"源用户为"<<login_name<<",目标用户"<<target_name<<"仍未登录，无法发起私聊\n";
			string sendstr = "源用户为" + login_name + ",目标用户" + target_name + "仍未登录，无法发起私聊";
			send(conn, sendstr.c_str(), sendstr.length(), 0);
		}else {
			// 找到目标
			cout<<"源用户"<<login_name<<"向目标用户"<<target_name<<"发起的私聊即将建立";
            cout<<",目标用户的套接字描述符为"<<name_sock_map[target]<<endl;
			target_conn = name_sock_map[target_name];
		}
	}else if (str.find("content:") != str.npos) {
		// 私聊信息
		if(target_conn == -1){
			cout<<"找不到目标用户"<<target_name<<"的套接字，将尝试重新寻找目标用户的套接字\n";
			if(name_sock_map.find(target_name)!=name_sock_map.end()){
                target_conn=name_sock_map[target_name];
                cout<<"重新查找目标用户套接字成功\n";
            }
            else{
                cout<<"查找仍然失败，转发失败！\n";
            }
		}
		string recvstr(str);
		string sendstr = recvstr.substr(8);
		cout<<"用户"<<login_name<<"向"<<target_name<<"发送:"<<sendstr<<endl;
        sendstr="["+login_name+"]:"+sendstr;
        send(target_conn,sendstr.c_str(),sendstr.length(),0);
	} else if(str.find("group:") != str.npos) {
		// 加入群聊
		string recvstr(str);
		string numstr = recvstr.substr(6);
		group_num = stoi(numstr);
		cout << "用户" << login_name << "加入群聊: " << numstr << endl;
		{
			lock_guard<mutex> lck(group_mutex);
			group_map[group_num].insert(conn);
		}
	} else if(str.find("gr_message:") != str.npos){
		// 发送群聊
		string sendstr(str);
		sendstr = sendstr.substr(11);
		sendstr = "[" + login_name + "]:" + sendstr;
		cout << "群聊信息: " << sendstr << endl;
		for(auto i: group_map[group_num]){
			if(i!=conn){
				send(i, sendstr.c_str(), sendstr.length(), 0);
			}
		}
	} else if(str.find("cookie:") != str.npos) {
		// cookie
		string cookie = str.substr(7);
		// redis query
		string redisstr = "hget " + cookie + " name";
		redisReply *r = (redisReply *)redisCommand(redis_target, redisstr.c_str());
		string sendres;
		if(r->str){
			cout << "查询redis结果: " << r->str << endl;
			sendres = r->str;
		}else {
			sendres = "NULL";
		}
		send(conn, sendres.c_str(), sendres.length()+1, 0);
	}
	
	// 更新 conn
	get<0>(info) = login_flag;
	get<1>(info) = login_name;
	get<2>(info) = target_name;
	get<3>(info) = target_conn;
	get<4>(info) = group_num;

}