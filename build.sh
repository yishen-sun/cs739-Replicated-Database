if [ -z "$1" ]; then
    echo "The first argument is empty."
    mkdir -p build
    cd build
    cmake ../src
    make
    cd ..
else
    echo "The first argument is not empty."
    mkdir -p build
    cd build
    cmake ../src -D USE_REDIS=ON
    make
    cd ..
fi


mkdir -p release
cp build/cli_client release/cli_client
cp build/kvraft_grpc_server release/kvraft_grpc_server
cp build/throughtput release/throughtput
