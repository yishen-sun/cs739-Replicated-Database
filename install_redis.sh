sudo apt install -y redis-server
export MY_INSTALL_DIR=$HOME/.local
export PATH="$MY_INSTALL_DIR/bin:$PATH"
# Clone the project
git clone https://github.com/Cylix/cpp_redis.git
# Go inside the project directory
cd cpp_redis
# Get tacopie submodule
git submodule init && git submodule update
# Create a build directory and move into it
mkdir -p build && cd build
# Generate the Makefile using CMake
cmake .. -DCMAKE_BUILD_TYPE=Release
# Build the library
make
# Install the library
sudo make install

cd tacopie
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
sudo make install

cd ../../..