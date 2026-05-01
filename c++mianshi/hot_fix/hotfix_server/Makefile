CXX=g++
CXXFLAGS=-O2 -Wall -pthread
# 动态库编译参数
SO_FLAGS=-fPIC -shared -O2

# 主程序 需要链接 -ldl
server: server.cpp logic_api.h
	$(CXX) $(CXXFLAGS) -o server server.cpp -ldl

# 编译旧战斗动态库
libbattle_old.so: battle_old.cpp
	$(CXX) $(SO_FLAGS) -o libbattle_old.so battle_old.cpp

# 编译新战斗动态库
libbattle_new.so: battle_new.cpp
	$(CXX) $(SO_FLAGS) -o libbattle_new.so battle_new.cpp

# 一次性编译所有
all: server libbattle_old.so libbattle_new.so

clean:
	rm -f server *.so