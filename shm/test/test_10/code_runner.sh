g++ -c SharedMemoryIPC.cpp -o SharedMemoryIPC.o -pthread
g++ writer.cpp SharedMemoryIPC.o -o writer -pthread
g++ reader.cpp SharedMemoryIPC.o -o reader -pthread

# ./writer
# ./reader
# ./reader