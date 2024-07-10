# Bitsy

Bitsy is an itsy-bitsy bit vector with rank and select support, which has a
space overhead of 3.40% in its default (fastest) configuration.

## How to build

The requirements to build Bitsy are a C++20 compiler (GCC/Clang), CMake
(v3.16+) and a build system (Ninja, GNU Make, etc.). To build Bitsy follow the
steps below:
```shell
# Fetch a copy of Bitsy if not already present
git clone https://github.com/dsalwasser/Bitsy.git
cd Bitsy
# Actually build Bitsy
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### PDEP Instructions

By default, the PDEP instruction is used. If Bitsy is running on a machine that
has a slow PDEP instruction, then we recommend disabling this feature by adding
the CMake flag `-DBITSY_USE_PDEP=Off`, as alternative implementations will be
faster. Processors with a slow PDEP instruction are for example Zen 1 and Zen 2
generation AMD processors. On the other hand, all Intel processors and AMD
processors after the Zen 2 generation have a fast PDEP instruction.

### Huge Pages

Furthermore, huge pages are used by default to improve performance. Thus, the
implementation first tries to allocate (2 MiB sized) huge pages and allocates
normal pages as a fallback. However, if you do not want to use huge pages for
some reason, this can be switched off with the additional CMake flags
`-DBITSY_HUGE_PAGES=Off`.

### Cross-Compilation

Also, the compiler flag `-mtune=native` is used by default. If Bitsy is
not compiled on the machine on which it is executed, the flag can be disabled
by adding the additional CMake flag `-DBUILD_WITH_MTUNE_NATIVE=Off`. However,
we recommend compiling Bitsy on the machine on which it is executed using
`-mtune=native` to improve performance.

## How to use

We provide the `ads_programm` application to answer queries for a bit vector
stored in a file and to write the corresponding answers to an output file. To
use this application, first compile Bitsy by following the above steps and then
run:
```shell
./build/apps/ads_programm <input_file> <output_file>
```
