# Build Instructions

- **Linux**
```
$ cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
$ cmake --build build
```
- Windows
- WSL

This will create two executables in `build/`, `text-search` and `text-search-test`.

# Running Tests

```
$ build/text-search-test -d data -f common-words.txt
```

# Implementations Overview

- `candidate_v1`:
  Loops through each query and then through each character of the text. If a character matches the first character of
  the query,
  its index (a candidate) is added to a vector. After all candidates for all queries are collected, each are checked
  character-by-character.

- `candidate_v2`:
  Same as `candidate_v1` but uses a pre-allocated int array per query the size of the text.

- `candidate_v3`:
  To save storage and improve cache utilization, a bit mask is used instead of an int array.

- `candidate_v4`:
  This creates a huge bit mask for all queries together instead of creating one per query.

- `hash`:
  A rolling hash approach, hard to parallelize.

- `std`:
  An implementation using C++ standard library functions used to check for correctness.

- `candidate_openmp_v1`:
  Parallelization of `candidate_v3` using OpenMP.

- `candidate_openmp_v2`:
  Parallelization of `candidate_v4` using OpenMP.

# Rolling Hash Implementation

1. **Define Prime Number**  
   In this implementation, we use a fixed prime number: `131`.

2. **Calculate Query Hash**  
   To compute the `query_hash`, each character of the query is multiplied by powers of the prime number:

```
    query_hash = query[0]*prime^(m-1) + query[1]*prime^(m-2) + ... + query[m-1]
```

This represents the query as a number in base `prime`.

3. **Compute Power for Rolling Hash**  
   Calculate `power = prime^(m-1)` based on the length of the query.  
   This is necessary to correctly remove the leftmost character when rolling the hash over the text.

4. **Compute Hash for the First Window of the Text**  
   Calculate the hash of the first substring of the text with the same length as the query.

5. **Rolling Hash Across the Rest of the Text**  
   For each subsequent position:

- Remove the leftmost character:
  ```
  window_hash -= text[i-1] * power
  ```
- Shift the hash:
  ```
  window_hash *= prime
  ```
- Add the new rightmost character:
  ```
  window_hash += text[i + m - 1]
  ```

6. **Check for Hash Matches**  
   If the rolling hash matches `query_hash`, optionally perform a character-by-character comparison to confirm the
   match.
