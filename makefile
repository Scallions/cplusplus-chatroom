
test_server: test_server.cpp server.cpp server.h
	g++ -std=c++11 test_server.cpp server.cpp -lpthread -o bin/test_server

clean:
	rm bin/test_server