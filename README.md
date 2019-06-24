# xdscribe

This code is part of the so called **inverse branch-and-bound technique** research. It solves the particular design centering problem from the diamonds cutting industry by a set of methods providing both the reference implementation of the inverse one and a relative performance comparison. See problem description and technique description sections below for further information.

All the source code in this repository is licensed under the GPLv3 license located in [LICENSE](LICENSE) file. Note that some external libraries are automatically downloaded and used having other licenses compatible with the main one. The data in `examples/cuts` and `examples/stones` directories have different licenses, see respective LICENSE files located there.

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
> V.H. Nguyen, J.-J. Strodiot: Computing a global optimal solution to a design centering problem. // Mathematical Programming, Vol. 53, pp 111-123. (1992)

and references therein.

The general `xdscribe` usage is as follows
```
xdscribe pattern_file contour_file stop_predicate inscriber_code [ tries [ direct_magic ] ]
```
where `pattern_file` and `contour_file` refer to pattern and contour models respectively, provided they are in [Wavefront .OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file) file format. Note that only triangular facets are supported! Look at the `examples` directory for some models to start with.

`stop_predicate` should be one of the following:
* `<target_precision>`, for example `1e-3`
* `v<target_value>`, for example `v0.5`. Note that this value should be reachable, otherwise `xdscribe` won't terminate!
* `s<seconds_to_run>`, for example `s20`.

Note that in any case the running time is limited to 5 hours! If you are ready to wait longer, feel free to modify default duration value in `solver/inscriber.h` file.

`inscriber_code` refers to a particular method to use. There are plenty of them, invoke `xdscribe` with no arguments to see the choices. For example, `gfhbrl` means graphic inverse algorithm with a flood-fill pattern convex decomposition, halfspaces minkowski sum rasterizer, contour rasterizer with bounding box enhanced facets rasterization and ray-combined inner region rasterization, lipschitzian accuracy estimation.

`tries` specifies the number of algorithm invocations within the single xdscribe run. This is useful for more precise time measurement.

`direct_magic` specifies a magic epsilon parameter of DIRECT algorithm if it is used. For an explanation refer to
> D.R. Jones, C.D. Perttunen, B.E. Stuckman: Lipschitzian optimization without the Lipschitz constant. // Journal of Optimization Theory and Applications, Vol. 79, Issue 1, pp 157-181. (1993)

For example, to test run after building under `build` subdirectory in Unix command line type
```
./xdscribe ../examples/box_12.obj ../examples/tetrahedron_4.obj 1e-3 gfhbrl
```

### Inverse branch-and-bound technique description

The main idea of the inverse technique is to step away from the conventional optimization framework based on a user-defined function computing the objective value at a given point of the domain. Instead we depend on a procedure evaluating the domain region with objective bounded by a given value.

Within the branch-and-bound approach this means that we branch on the objective value, like the well-known [bisection method](https://en.wikipedia.org/wiki/Bisection_method), than bound the region directly by pruning irrelevant area, and repeat these steps ensuring the region to be non-empty. The graphical inverse method implemented by xdscribe represents the regions by images on a volumetric raster consisting of inner, boundary and outer [voxels](https://en.wikipedia.org/wiki/Voxel). The images once acquired make region bounding and checking non-emptiness trivial tasks.

For a design centering problem it remains to explain the procedure to obtain a region image with bounded objective value. This is done using the voxelization of contour [erosion](https://en.wikipedia.org/wiki/Erosion_(morphology)) by inverted pattern which means filling the voxels too close to contour and thus not containing pattern placements of an appropriate scale. The erosion itself is implemented by the well-known [Minkowski sum](https://en.wikipedia.org/wiki/Minkowski_addition) algorithms.
