CXX=g++ -g -std=c++11
CXXFLAGS=-O1 -Wall -pthread
# 动态库编译参数
SO_FLAGS=-fPIC -shared -O1

# 主程序 需要链接 -ldl
server: server.cpp logic_api.h
	$(CXX) $(CXXFLAGS) -o server server.cpp -ldl

# 主程序 需要链接 -ldl
server_test: test.cpp
	$(CXX) $(CXXFLAGS) -o server_test test.cpp -ldl

# 编译旧战斗动态库
libbattle_old.so: battle_old.cpp
	$(CXX) $(SO_FLAGS) -o libbattle_old.so battle_old.cpp

# 编译新战斗动态库
libbattle_new.so: battle_new.cpp
	$(CXX) $(SO_FLAGS) -o libbattle_new.so battle_new.cpp

# 一次性编译所有
all: server_test server libbattle_old.so libbattle_new.so

clean:
	rm -f server server_test *.so