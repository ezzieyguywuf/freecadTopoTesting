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
  * 10 x 20 x 30 cube is created (not extruded).
  * A 5 x 5 x 5 cube is created with it's Top Face co-planar with the Top Face of the
first cube. It is roughly centered on the top face
  * The smaller cube is `Cut` from the larger one
  * Three `txt` files are written containing `Dump` data for the large box, the small
one, and the result box of the `Cut` operation
2. `runCase2()` same as #1 except the first box is extruded from a 2D square.
**Note** For both `runCase1` and `runCase2` you can edit the bottom of
`cutBoxAndShowThings` function to control what gets printed to `std::cout` as well as
what gets written to files
3. `runCase3()` this is my implementation of the opencascade sample code provided for
   `TNaming`. I've added a lot of commentary, some of it rambling, and I've also renamed
   their variables to things that make more sense to me. This may be a good starting point
   if you need to sort of figure out what this `TNaming` thing is and how it works.
4. `runCase4()` this is a straight up copy-and-paste of the opencascade sample code. It
   should compile and run properly. I've begun to extend it to show some of the geometery
   as well as prototype how we may use this in FreeCAD

# python parsers
`DumpDataViewer.py` is intended to be used on a txt file that contains the full output of
`BRepTools::Dump`. It will pull out the Face data near the bottom. If more than one txt
file is provided, it would format the output so that you can compare the data
side-by-side.

`ShapeDataFormatter.py` is intended to be used with a txt files that contains the output
of the `printShapeInfos` command. This `printShapeInfos` takes three `TopoDS_Shape`s, uses
`TopExp_MapShapes` to traverse the `TopoDS_FACE`s in the `Shape`, and writes out the
geometric location and the direction of the perpendicular vector to the Face. This gives
enough information to uniquely identify each Face. The python script takes this output and
formats it so that they are side-by-side, for easier comparison.
