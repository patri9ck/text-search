# Very Simple Sequential Search Implementation (VSSSI)
1. Find candidates in text by searching for the first and last symbol of the query.
   * All candidates are saved in a bit mask to save memory.
2. Loop through the bit mask and check each candidate manually.

A candidate is always the leading index of the query in the whole text (either as an int or encoded in a bit mask).

The results are compared to a simple implementation using the C++ standard library.

## Building and Running
```
mkdir build
cd build
cmake ..
cmake --build .
./sequential-text-search ../pg77274.txt 'be found wedded to the perspective of the'
```
