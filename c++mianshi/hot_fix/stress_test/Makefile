all: server libbattle_old.so libbattle_new.so

server: http_server.cpp
	g++ -g -o server http_server.cpp -pthread -ldl -O2 -std=c++11

libbattle_old.so: battle_old.cpp
	g++ -g -fPIC -shared -o libbattle_old.so battle_old.cpp -std=c++11

libbattle_new.so: battle_new.cpp
	g++ -g -fPIC -shared -o libbattle_new.so battle_new.cpp -std=c++11

clean:
	rm -rf server *.so