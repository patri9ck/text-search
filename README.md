# Build Instructions

- **Linux**

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

This will create two executables in `build/`, `text-search` and `text-search-test` (or`text-search.exe` and
`text-search-test.exe` on Windows, respectively).

# Using the Command Line Tool

The command line tool `text-search` provides the best implementations using OpenMP, MPI, OpenCL (safe or unsafe) or
simply a single thread (sequential).

Example call:

```
build/text-search -i openmp -d data -f README.md -q text -q search
```

This uses the best OpenMP implementation, loads all files in the directory `data/` and the file `README.md` to look
through and searches for the words
`text` and `search`. Run `build/text-search --help` for a list of all options.

# Running Tests

Example call:

```
build/text-search-test -i candidate_v3 -d data -f common-words.txt -c
```

This will load all books from the directory `data/`, all queries in the file `common-words.txt`, run the `candidate_v3`
implementation and
test it against the reference implementation. Run `build/text-search-test --help` for an overview of all options.

# Creating Plots

To create plots and CSV files, `benchmark.py` exists which calls `text-search-test`. It stores its results in `doc/`.

Example call:

```
python3 benchmark.py -i openmp -e build/text-search-test -m queries -q common-words.txt -b data
```

Run `python3 benchmark.py --help` for an overview of all options.

# Downloading Ebooks from Project Gutenberg

To download the top 100 ebooks from Project Gutenberg into `data/`, run:

```
python3 download-gutenberg-ebooks.py
```

# Implementations Overview

- `std`:
  Uses the standard library, e.g. `std::string::find()`, and is used to check for correctness.

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

- `std_openmp`
  Parallelization of `std` using OpenMP.

- `candidate_openmp_v1`:
  Parallelization of `candidate_v3` using OpenMP.

- `candidate_openmp_v2`:
  Parallelization of `candidate_v4` using OpenMP.

- `hash_openmp`

- `candidate_mpi`

- `candidate_opencl_v1`

- `candidate_opencl_v2`

- `candidate_opencl_v3`
