echo "Running CMake configuration..."
cmake -S . -B build

echo "Building the project..."
make -C build

echo "Running the project..."
./build/bin/Server
