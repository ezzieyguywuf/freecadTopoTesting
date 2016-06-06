# freecadTopoTesting
A bare-bones OpenCascade project that is attempting to resolve the Topological Naming
issue in FreeCAD

# installation

    git clone https://github.com/ezzieyguywuf/freecadTopoTesting.git
    cd freecadTopoTesting

**Note** You need to edit `CMakeLists.txt` and point it to the proper location for your
opencascade installation. Specifically, the `include_directories` needs to point to the
directory where your opencascade include files are, and `link_directories` needs to point
to the directory where the opencascade `*.so` files are.

    mkdir Build
    cd Build
    cmake ..
    make

# execution
The `make` command will create a `bin` directory and dump the executable there. So, you
can run the program by doing this from the `Build` directory:

    ./bin/occTest

# run cases
The `main` function is super simple, and essentially calls one or more of some different
'run case's that I have defined. Here is a brief description of each:

1. `runCase1()`
..1. 10 x 20 x 30 cube is created (not extruded).
..2. A 5 x 5 x 5 cube is created with it's Top Face co-planar with the Top Face of the
first cube. It is roughly centered on the top face
..3. The smaller cube is `Cut` from the larger one
..4. Three `txt` files are written containing `Dump` data for the large box, the small
one, and the result box of the `Cut` operation
2. `runCase2()` same as #1 except the first box is extruded from a 2D square.
**Note** For both `runCase1` and `runCase2` you can edit the bottom of
`cutBoxAndShowThings` function to control what gets printed to `std::cout` as well as
what gets written to files
3. `runCase3()` this is my implementation of the opencascade sample code provided for
   `TNaming`. I've added a lot of commentary, some of it rambling, and I've also renamed
   their variables to things that make more sense to me. This may be a good starting point
   if you need to sort of figure out what this `TNaming` thing is and how it works.
4. `runCase4()` this is a straight up copy-and-paste of the opencascade sample code. I
   have not compiled or run this code yet, just including it because it helps to look at
   their 'clean' vanilla example sometimes, without all my extra commentary.
