protoc --cpp_out=. user.proto
g++ benchmark.cpp user.pb.cc -o benchmark \
  -I/usr/include/jsoncpp -I/usr/include/libxml2 -lprotobuf -ljsoncpp -lxml2 -std=c++11 -O2