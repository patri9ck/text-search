## Build Instructions

### Windows

Install Visual Studio with (at least) the following components:

- MSVC-Buildtools für x64/x86 (neueste Version)
- C++-CMake-Tools für Windows
- Git für Windows
- Windows 11 SDK (10.0.22621.0)

In case you have Visual Studio already installed and a component is missing, open the **Visual Studio Installer** and
choose **Ändern**.

To use OpenCL, the OpenCL SDK from Khronos is needed. Binaries can be downloaded from
the [official repository](https://github.com/KhronosGroup/OpenCL-SDK/releases).
To make things easier, this repository already bundles binaries for the OpenCL SDK v2025.07.23 in the
`bin/OpenCL-SDK-v2025.07.23-Win-x64/`
directory. Later, when building the project, we can point CMake to this directory.

For MPI, simply
install [msmpisdk.msi and msmpisetup.exe](https://learn.microsoft.com/de-de/message-passing-interface/microsoft-mpi) to
use Microsoft MPI. **Both must be installed.**

Then, open **Developer PowerShell for VS** and change the directory to a location where you have sufficient permission,
e.g.:

```
cd ~
```

Next, clone the repository.

```
git clone https://github.com/KN-PACO/text-search.git
cd text-search
```

Finally, build the project using CMake:

```
cmake -S . -B build -DOpenCL_ROOT="$pwd\bin\OpenCL-SDK-v2025.07.23-Win-x64"
cmake --build build --config Release
```

This will create two executables in `build/Release/`, `text-search.exe` and `text-search-test.exe`.

In case everything fails, we provide binaries in the `bin/` directory.

### Linux

This requires CMake for building, an MPI implementation (e.g. Open MPI) and, for OpenCL, an ICD loader (e.g. ocl-icd) as
well as a runtime (e.g. AMD CLR for AMD).

Get started by cloning the repository:

```
git clone https://github.com/KN-PACO/text-search.git
cd text-search
```

Then, build it using CMake:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

This will create two executables in `build/`, `text-search` and `text-search-test`.

## Using the Command Line Tool

The command line tool `text-search` provides the best implementations using OpenMP, MPI, OpenCL (safe or unsafe), or
simply a single thread (sequential).

Example call (Windows):

```
build/Release/text-search.exe -i openmp -d data -f README.md -q text -q search
```

Example call (Linux):

```
build/text-search -i openmp -d data -f README.md -q text -q search
```

This uses the best OpenMP implementation, loads all files in the directory `data/` and the file `README.md` to look
through and searches for the words
`text` and `search`. Use the `--help` option for a list of all options.

For MPI, simply use the MPI implementation and wrap the command using `mpiexec`.

Example call (Windows):

```
mpiexec -n 8 build/Release/text-search.exe -i mpi -d data -f README.md -q text -q search
```

Example call (Linux):

```
mpiexec -n 8 build/text-search -i mpi -d data -f README.md -q text -q search
```

## Running Tests

Example call (Windows):

```
build/Release/text-search-test.exe -i candidate_v3 -d data -f common-words.txt -c
```

Example call (Linux):

```
build/text-search-test -i candidate_v3 -d data -f common-words.txt -c
```

This will load all books from the directory `data/`, all queries in the file `common-words.txt`, run the `candidate_v3`
implementation and
test it against the reference implementation. Run `--help` for an overview of all options.

Again, to use MPI, choose an MPI implementation and wrap the command using `mpiexec`.

## Creating Plots

To create plots and CSV files, `benchmark.py` exists which calls `text-search-test`. It stores its results in `doc/`.

Example call (Windows):

```
python3 benchmark.py -i openmp -e build/Release/text-search-test.exe -m queries -q common-words.txt -b data
```

Example call (Linux):

```
python3 benchmark.py -i openmp -e build/text-search-test -m queries -q common-words.txt -b data
```

Use the `--help` options for an overview of all options.

Existing plots and CSV files can already be found in the `doc/` directory.

## Downloading Ebooks from Project Gutenberg

To download the top 100 ebooks from Project Gutenberg into `data/`, run:

```
python3 download-gutenberg-ebooks.py
```

The repository already includes the top 100 from December 2025.

## Implementations Overview

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

- `hash_v1`:
  Rabin-Karp rolling-hash implementation.

- `hash_v2`:
  Groups queries by length instead and uses an unordered map as a hash table.

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
