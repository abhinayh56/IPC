# g++ -c SharedMemoryBus.cpp -o SharedMemoryBus.o -pthread

# g++ -std=c++17 writer.cpp -o writer -lpthread
# g++ -std=c++17 reader.cpp -o reader -lpthread

# ./writer 1    # in one terminal
# ./writer 2    # in another terminal
# ./reader    # in another (run multiple if needed)

# sudo setcap cap_ipc_lock=eip ./writer
# sudo setcap cap_ipc_lock=eip ./reader
# sudo setcap -r ./writer
# sudo setcap -r ./reader

g++ -std=c++17 test.cpp -o test -lpthread