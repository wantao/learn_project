# 构建镜像
docker build -t game_server:v1 .

# 启动容器：端口映射 + 日志挂载 + 开机自启
docker run -d \
  --name game_server \
  --restart=always \
  -p 8080:8080 \
  -p 27015:27015/udp \
  -v /home/learn_project/c++mianshi/docker_test/logs:/game_server/logs \
  game_server:v1

# 查看容器运行状态
docker ps | grep game_server

# 查看容器实时日志
docker logs -f game_server

# 进入容器内部
docker exec -it game_server /bin/bash

# 重启/停止容器
docker restart game_server
docker stop game_server