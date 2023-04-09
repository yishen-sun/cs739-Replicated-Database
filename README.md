# cs739-Replicated-Database

```
sh install_grpc.sh
sh install_redis.sh
```
to build a basic version
```
sh build.sh basic
```
to build a redis version
```
sh build.sh redis
```

```
sudo systemctl start redis-server
sudo systemctl status redis-server
sudo systemctl stop redis-server
```
```
redis-cli
DBSIZE
flushall
```
basic version
```
./release/cli_client 0.0.0.0:50001
./release/kvraft_grpc_server server_a 0.0.0.0:50001 ./basic_server_config.txt
./release/kvraft_grpc_server server_b 0.0.0.0:50001 ./basic_server_config.txt
./release/kvraft_grpc_server server_c 0.0.0.0:50001 ./basic_server_config.txt
```
redis version
```
./release/cli_client amd1310.utah.cloudlab.us:50001
./release/kvraft_grpc_server server_a ms1310.utah.cloudlab.us:50001 ./redis_server_config.txt
./release/kvraft_grpc_server server_b ms1317.utah.cloudlab.us:50001 ./redis_server_config.txt
./release/kvraft_grpc_server server_c ms1343.utah.cloudlab.us:50001 ./redis_server_config.txt
```