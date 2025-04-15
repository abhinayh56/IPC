# Compile writer
```
g++ writer.cpp -o writer -pthread
```

# Compile reader
```
g++ reader.cpp -o reader -pthread
```

# Run writer
Open a terminal and run the following program to execute the writer
```
./writer
```

You will see the following output on console
```
Writer wrote: Hello from writer!
```

# Run reader
Open another terminal and run the following program to execute the reader
```
./reader
```

You will see the following output on console
```
Reader got: Hello from writer!
```

**Note:** 

1. Execute write program before the reader program, otherwise it will show segmentation error as following
```
Segmentation fault (core dumped)
```

2. This prgram runs in non-blocking mode