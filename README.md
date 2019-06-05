# xdscribe

This code is part of the so called **inverse branch-and-bound technique** research. It solves the particular design centering problem from the diamonds cutting industry by a set of methods providing both the reference implementation of the inverse one and a relative performance comparison. See problem description and technique description sections below for further information.

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

Problem solved by xdscribe is as follows. Given a rough stone model(contour) and a target shape(pattern) find the largest placement of the pattern inside the contour preserving orientation, i.e. allowing only translation and scaling of the shape. For a further problem description see
> V.H. Nguyen, J.-J. Strodiot: Computing a global optimal solution to a design centering problem. // Mathematical Programming, Vol. 13, pp 271-369. (2004)
and references therein.

The general `xdscribe` usage is as follows
```
xdscribe contour_file pattern_file stop_predicate inscriber_code [ tries [ direct_magic ] ]
```
where `contour_file` and `pattern_file` refer to contour and pattern models respectively, provided they are in [Wavefront .OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file) file format. Note that only triangular facets are supported! Look at the `examples` directory for some models to start.

`stop_predicate` should be one of the following:
* `<target_precision>`, for example `1e-3`
* `v<target_value>`, for example `v0.5`. Note that this value should be reachable, otherwise `xdscribe` won't terminate!
* `s<seconds_to_run>`, for example `s20`.

`inscriber_code` refers to a particular method to use. There are plenty of them, invoke `xdscribe` with no arguments to see the choices. For example, `gfhbr` means graphic algorithm with a flood-fill pattern convex decomposition, halfspaces minkowski sum rasterizer and polytope rasterizer with bbox-enhanced facets rasterization and ray-combined inner region rasterization.

`tries` specifies the number of algorithm invocations within the single xdscribe run. This is useful for more precise time measurement.

`direct_magic` specifies a magic epsilon parameter of DIRECT algorithm if it is used. For an explanation refer to
> D.R. Jones, C.D. Perttunen, B.E. Stuckman: Lipschitzian optimization without the Lipschitz constant. // Journal of Optimization Theory and Applications, Vol. 79, Issue 1, pp 157-181. (1993)

For example, to test run after building under `build` subdirectory in Unix command line type
```
./xdscribe ../examples/tetrahedron_4.obj ../examples/box_12.obj 1e-3 gfhbr
```

### Inverse branch-and-bound technique description
