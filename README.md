# xdscribe

This code is part of the so called **inverse branch-and-bound technique** research. It solves the particular design centering problem from the diamonds cutting industry by a couple of methods providing both the reference implementation of the inverse one and a relative performance comparison. See technique description and problem description sections below for further information.

All the code in this repository is licensed under the GPLv3 license located in [LICENSE](LICENSE) file. Note that some external libraries are automatically downloaded and used having other licenses compatible with the main one.

### Compiling

This is a C++17 CMake project. So normally you should install your platform's recent C++ development tools along with [git](https://git-scm.com/) and [CMake](https://cmake.org), then clone this repository and build similarly to other CMake-based projects. For example, in Unix command line you should type something like that:

```
git clone https://github.com/kserz/xdscribe.git
cd xdscribe
mkdir build
cd build
cmake ..
cmake --build .
```
This will download all the dependencies under `extern/cache` directory and build xdscribe executable. Then run `xdscribe` with no arguments to see the help message.

### Problem description and xdscribe usage

### Inverse branch-and-bound technique description
