# cs739-Replicated-Database

```
sh install_grpc.sh
sh install_redis.sh
sh build.sh
```

```
sudo systemctl start redis-server
sudo systemctl status redis-server
sudo systemctl stop redis-server
```

```
./release/cli_client amd1310.utah.cloudlab.us:50001
./release/kvraft_grpc_server server_a amd1310.utah.cloudlab.us:50001 ./server_config.txt
./release/kvraft_grpc_server server_b amd1317.utah.cloudlab.us:50001 ./server_config.txt
./release/kvraft_grpc_server server_c amd1343.utah.cloudlab.us:50001 ./server_config.txt
```