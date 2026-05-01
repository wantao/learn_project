g++ -shared -fPIC battle.cpp -o libbattle.so -std=c++11
g++ test.cpp -o test -L. -lbattle -std=c++11
