#include <thread>
#include <iostream>
#include <unistd.h>

using namespace std;
void print() {
	for(int i=1;i<=10;i++){
		cout << i << endl;
		sleep(1);
	}
}

int main() {
	thread t1(print), t2(print);
	t1.join();
	t2.join(); // 需要join不然直接退出主线程会导致其他线程强制退出
	return 0;
}