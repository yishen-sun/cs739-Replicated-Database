if [ "$1" = "basic" ]; then
    echo "The second argument is basic."
    mkdir -p build
    cd build
    cmake ../src
    make
    cd ..
elif [ "$1" = "redis" ]; then
    echo "The second argument is redis."
    mkdir -p build
    cd build
    cmake ../src -D USE_REDIS=ON
    make
    cd ..
else
    echo "You should input redis or basic as the second argument."
fi


mkdir -p release
cp build/cli_client release/cli_client
cp build/kvraft_grpc_server release/kvraft_grpc_server
cp build/throughtput release/throughtput
