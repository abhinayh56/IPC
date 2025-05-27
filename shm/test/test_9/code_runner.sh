g++ -o SharedMemoryMultiRW SharedMemoryMultiRW.cpp -lpthread -lrt

# ./SharedMemoryMultiRW writer "Hello from writer 1"
# ./SharedMemoryMultiRW writer "Hello from writer 2"
# ./SharedMemoryMultiRW reader
# ./SharedMemoryMultiRW reader
