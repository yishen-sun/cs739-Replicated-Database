
mkdir -p build
cd build
cmake ../src
make -j 4
cd ..

mkdir -p release
cp build/cli_client release/cli_client
cp build/server release/server
