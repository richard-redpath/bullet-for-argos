# Bullet for ARGoS

## Introduction
This is a plugin which brings the bullet physics engine to the ARGoS robot simulator. It allows entities to be declared in XML files which can then be loaded and instantiated inside simulations.

## Build instructions
Building is done in the same manner as any CMake project. First, create a directory to hold the compiled source and change into it.
```
mkdir build && cd build
```

Next, use cmake to generate a Makefile and invoke make to build the project.
```
cmake .. && make -j 8
```

The compiled library will now be available in the `lib` directory of our build directory.

## Publication

More details and format description can be found in "Introducing a 3D Physics Simulation Plugin for the ARGoS Robot Simulator" [DOI 10.1007/978-3-319-40379-3_27](http://dx.doi.org/10.1007/978-3-319-40379-3_27)
