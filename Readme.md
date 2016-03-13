#Bullet for ARGoS

##Introduction
This is a plugin which brings the bullet physics engine to the ARGoS robot simulator. It allows entities to be declared in XML files which can then be loaded and instantiated inside simulations.

##Build instructions
Building is done in the same manner as any CMake project. First, create a directory to hold the compiled source and change into it.
```
mkdir build && cd build
```

Next, use cmake to generate a Makefile and invoke make to build the project.
```
cmake .. && make -j 8
```

The compiled library will now be available in the `lib` directory of our build directory.
