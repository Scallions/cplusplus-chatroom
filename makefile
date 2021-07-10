all: test_server test_client

test_client: test_client.cpp client.cpp client.h global.h
	g++ -std=c++11 test_client.cpp client.cpp -lpthread -o bin/test_client

test_server: test_server.cpp server.cpp server.h global.h
	g++ -std=c++11 test_server.cpp server.cpp -lpthread -o bin/test_server

clean:
	rm bin/test_server
	rm bin/test_client