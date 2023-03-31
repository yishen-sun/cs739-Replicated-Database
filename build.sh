
mkdir -p build
cd build
cmake ../src
make
cd ..

mkdir -p release
cp build/cli_client release/cli_client
cp build/kvraft_grpc_server release/kvraft_grpc_server
cp build/throughtput release/throughtput
