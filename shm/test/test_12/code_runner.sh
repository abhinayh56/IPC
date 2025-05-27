g++ -c SharedMemoryBus.cpp -o SharedMemoryBus.o -pthread

g++ writer.cpp SharedMemoryBus.o -o writer -pthread
g++ reader.cpp SharedMemoryBus.o -o reader -pthread

g++ -std=c++17 writer.cpp SharedMemoryBus.o -o writer -lpthread
g++ -std=c++17 reader.cpp SharedMemoryBus.o -o reader -lpthread

# ./writer 1    # in one terminal
# ./writer 2    # in another terminal
# ./reader    # in another (run multiple if needed)
