yum install -y gcc gcc-c++ cmake3 make
yum install -y boost boost-devel
yum install -y protobuf protobuf-devel
yum install -y mysql-devel hiredis-devel
wget https://github.com/protocolbuffers/protobuf/releases/download/v21.12/protobuf-all-21.12.tar.gz
tar -zxvf protobuf-all-21.12.tar.gz
cd protobuf-21.12/
./configure
make -j4
make install
ldconfig
cd ..
wget https://github.com/Kitware/CMake/releases/download/v3.28.1/cmake-3.28.1-linux-x86_64.tar.gz
tar -xzvf cmake-3.28.1-linux-x86_64.tar.gz -C /opt
ln -sf /opt/cmake-3.28.1-linux-x86_64/bin/cmake /usr/local/bin/cmake
cmake --version
yum remove -y hiredis hiredis-devel
git clone https://github.com/redis/hiredis.git
cd hiredis
make -j4
make install
cd ..
echo "/usr/local/lib64" >> /etc/ld.so.conf.d/local.conf && ldconfig
