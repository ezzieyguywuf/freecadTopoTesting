<pre>
/*********************************************************************************
*   Copyright (C) 2016 Wolfgang E. Sanyer (ezzieyguywuf@gmail.com)               *
*                                                                                *
*   This program is free software: you can redistribute it and/or modify         *
*   it under the terms of the GNU General Public License as published by         *
*   the Free Software Foundation, either version 3 of the License, or            *
*   (at your option) any later version.                                          *
*                                                                                *
*   This program is distributed in the hope that it will be useful,              *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of               *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                *
*   GNU General Public License for more details.                                 *
*                                                                                *
*   You should have received a copy of the GNU General Public License            *
*   along with this program.  If not, see http://www.gnu.org/licenses/           *
******************************************************************************** */
</pre>
# What is TNaming?

TNaming is an opencascade (henceforth referred to as occ) 'Package'. It provides a set of
tools for implementing a Topological Naming structure using the `TDF_Data` class. What is
`TDF_Data`? It is the class used in the TDF 'Package' for storing and manipulating a 'Data
Framework' (not sure what the T stands for, I think Transient). The occ Documentation does
a good job explaining the Data Framework and how it works, but here's a few highlights.

## occ Data Framework
Note: Please refer to the occ Documentation for a more detailed explanation.
[Here](http://www.opencascade.com/doc/occt-7.0.0/overview/html/occt_user_guides__ocaf.html#occt_ocaf_3)
is a direct link. It is under the 'OCAF' section of the Documentation, but fear not
because the Data Framework can be used outside the context of the OCAF thing.

So, the way that occ's TDF package manages Data in it's Data Framework is by creating
nodes in a tree, all stored in an instance of `TDF_Data`. Each node has a unique Tag used
for identifying it in the Data Framework. These tags are strings, typically of the form
"0:i:j:etc..." where each number represents a branch in the tree. For example, "0:1"
refers to the first node under the 'Root' of the tree, "0:2" refers to the second node
under the Root, "0:2:1" refers to the first child under the second node of the Root,
etc... Note that every node in the tree can eventually trace it's origins back to the
Root, i.e. "0". 

Each of these Tag's points to a discrete TDF_Label in the Data Framework. The `TDF_Label`,
in turn, contains zero or more `TDF_Attribute`s, which themselves contain actual data
that you may be interested in.

Why the distinction between Tag and `TDF_Label` and `TDF_Attribute`? Well, I can't speak to
the original intent of the occ devs, but it makes sense to me that you'd want a different
Tag and `TDF_Label`. Since the `TDF_Label` can, itself, contain many `TDF_Attribute`s that can
themselves contain a lot of data, it could be quite expensive to store an instance of each
TDF_Label that you create and care about. In fact, it would be wasteful because your
`TDF_Data` instance is already going to have a copy of each `TDF_Label`, so you'd be doubling
your memory footprint for no reason. Instead, store a value of the Tag itself, which is
nothing more than a string, and then use `TDF_Tool::Label(myDataFramework, tagToRetrieve,
emptyLabel, false)`to retrieve the `TDF_Label`, where `myDataFramework` is an instance of
`TDF_Data` where you are storing data, `tagToRetrieve` is a c-style string of the form
"0:1:n" etc.., and `emptyLabel` is `TDF_Label emptyLabel;` and will store the retrieved
label. Note that the final `false` is used to ensure the label isn't created if it doesn't
exist..

So what is an `TDF_Attribute`? Like I said earlier, it is the actual 'meat' of the  occ
Data Framework. The Tag is just an Address to a `TDF_Label`, which is just a construct
that holds attributes. The `TDF_Attribute` is a class that contains whatever information
it is that you care about. `TNaming_NamedShape` is a TDF_Attribute that is described in
the TNaming Package. Make sense? Lol, there appear to be many other `TDF_Attribute`s that
occ has defined, I guess expecting that they are common things that people will want to
store. For a full list, check
[here](http://www.opencascade.com/doc/occt-6.9.1/refman/html/class_t_d_f___attribute.html).
I use one of these for storing some text at each node: this allows me to store some useful
info about what is in the node, for debugging purposes. See
[TopoNamingHelper::AddTextToLabel](https://github.com/ezzieyguywuf/FreeCAD-1/blob/tnaming_ezziey/src/Mod/Part/App/TopoNamingHelper.h#L114)
for more info.

One more thing about `TDF_Attribute`s. A `TDF_Label` can only contain **one** of a given
`TDF_Attribute`. I don't really know why, I just know it's true. For this reason, every
`TDF_Attribute` that occ has defined has it's own unique GUID, which can be accessed via the
_class method_ `GetID()`. What I mean by 'class method' is that you do **_not_** do the
following:

    TNaming_NamedShape myNamedShape();
    Standard_GUID myGUID = myNamedShape.GetID() // WRONG! Don't do this!

Instead, what you want to do is simply

    Standard_GUID myGUID = TNaming_NamedShape::GetID() // Proper way to access class method

This GUID thing is important b/c it is the mechanism by which you grab a desired
TDF_Attribute from a TDF_Label. Consider it the equivalent of a Tag - it is the address by
which we access a `TDF_Attribute` in a `TDF_Label`.

Typical usage of `TDF_Label`, as well as retrieving an Attribute from a label:

    TDF_Label myLabel = fatherLabel.FindChild(1); // created label '1' if it didn't exist
    TDF_Label testLabel = fatherLabel.FindChild(1); // retrieved label '1'

    // TDF_TagSource is a nice way to let occ keep track of your Tag numbers.
    TDF_Label otherLabel = TDF_TagSource::NewChild(fatherLabel); // probably '2'

    Handle(TNaming_NamedShape)  myNamedShape;
    // this stores in myNamedShape a pointer to the TNaming_NamedShape in someLabel
    someLabel.FindAttribute(TNaming_NamedShape::GetID(), myNamedShape);

**NOTE:** The use of `TDF_TagSource` in this example in incorrect. In my experience, you
must either _always_ use `TDF_TagSource` or not use it at all. I have opted to always use
it, so that I don't have to worry about keeping track of things.

## TNaming_NamedShape

So, now that we know what a Tag, a `TDF_Label`, and a `TDF_Attribute` are, let's talk
about `TNaming_NamedShape`. `TNaming_NamedShape` inherits from `TDF_Attribute`: in other
words, it is a `TDF_Attribute`. It is actually a 'helper TDF_Attribute', if you will, that
writes and reads information from `TNaming_UsedShapes`. I don't know a ton about
`TNaming_UsedShapes` right now, but suffice to say that it:

1. Is a `TDF_Attribute`
2. Only exists on the root node of the the `TDF_Data` tree structure
3. Is created for you whenever you add a `TNaming_NamedShape` to your `TDF_Data` tree
4. Should never really need to be accessed directly (I think)
5. Most importantly, it is the grand-daddy of the whole Topological Naming algorithm and
mechanism that occ has implemented. It stores a list of all the `TopoDS_Shape`s that are
used in the evolution of the Shape.

The only method of the `TNaming_NamedShape` class I've found myself using is
`myNamedShape.Get()`, or more often `myNamedShape->Get()` as I'm typically dealing with a
pointer (or `Handle` in occ terms) to the `TNaming_NamedShape`. This method returns the
`TopoDS_Shape` which is stored inside the `TNaming_NamedShape` (or rather, the
`TopoDS_Shape` that the `TNaming_NamedShape` points to in the `TNaming_UsedShapes`
attribute, if you want to be technical).

You may be wondering "if the `TNaming_NamedShape` attribute is so important, how come
we're only using the Get method? How do we add information to the `TNaming_NamedShape`
attribute?" Good question! This is actually done using the `TNaming_Builder` class. As the
name implies, the `TNaming_Builder` class helps us to 'build' `TNaming_NamedShape`
attributes. Again, I won't deign to speak for the intent of the occ devs, but I imagine
the reason for the separate `TNaming_Builder` is to separate the entries into the
`TNaming_UsedShape` structure from the creation of those entries. For example, if you look
at the [occ
documentation](http://www.opencascade.com/doc/occt-7.0.0/overview/html/occt_user_guides__ocaf.html#occt_ocaf_5_3),
you'll notice that `TNaming_NamedShape` tracks the "evolution" of it's `TopoDS_Shape` in 5
different ways. `TNaming_Builder` makes it easier for us to insert these different
evolutions into the `TNaming_NamedShape` attribute, because there's some funky
behind-the-scenes stuff going on with some sort of `TNaming_Node` construct.

Things to know about `TNaming_Builder`:

1. Always use a `TNaming_Builder` to add `TopoDS_Shape` evolution information to a
`TNaming_NamedShape`
2. `TNaming_Builder` takes as an argument to its constructor the `TDF_Label` on which to
create the `TNaming_NamedShape` attribute.
  a. If no `TNaming_UsedShapes` exists in the root of the TDF_Data tree, it creates it
  b. If no `TNaming_NamedShape` exists at this `TDF_Label`, it creates one
  c. If a `TNaming_NamedShape` already exists at this `TDF_Label`, it creates some sort of
  backup, then clears all the data so that it can write the new data. I'm not certain, but
  I think this backup mechanism may be used by the `TNaming_UsedShape` thing and by the
  Topological Naming algorithm to figure things out.
3. Only use one `TNaming_Builder` per evolution type. In fact, I think it's safest to just
create a new `TNaming_Builder` any time you're dealing with a new operation.

Here's a typical use-case:

    TNaming_Builder myBuilder(myLabel);
    myBuilder.Generated( anEdge, aFace); // aFace was generated from anEdge

    TNaming_Builder myBuilder2(myLabel2);
    myBuilder2.Modify( origShape, newShape ); // origShape was somehowe modified (cut, fuse, etc...) into newShape

    TNaming_Builder myBuilder3(myLabel3);
    myBuilder3.Select( anEdge, aShape); // anEdge is an edge within the contex of aShape.

## TNaming_Selector

As described in [the occ
documentation](http://www.opencascade.com/doc/occt-6.9.1/refman/html/class_t_naming___selector.html#details)
for `TNaming_Selector`, it is the 'user interface for topological naming resources'.
`TNaming_Selector` must be used to 'select' a topological entity that you wish to keep a
constant reference to. OCC's topological naming algorithm will use the modeling history
that you store in the `TDF_Data` framework (by using `TNaming_Builder` to store
`TNaming_NamedShape` attributes) to resolve a reference to the selected topological
entity, even if the underlying geometry changes.

Take the following as an example:

1. A 10x10x10 Box is created used `BRepPrimAPI_MakeBox`
2. Edge '3' (as returned by `TopExp::MapShapes`) is filleted using
   `BRepFilletAPI_MakeFillet`
3. The height of the Box is increased to 15

Ostensibly, the application that is used to create, fillet, and modify the dimensions of
this box would like to retain the fillet on the same edge, regardless of any changes made
to the underlying box. `TopExp::MapShapes` cannot be relied upon, because 'edge 3' can
refer to different edges depending on how the Box is changed. This is done by first
'selecting' the appropriate Edge, and then putting sufficient information in the Data
Framework so that the topological naming algorithm knows what to do. This is perhaps best
explained by way of an example:

    BRepPrimAPI_MakeBox myBox(10., 10., 10.);
    Handle(TDF_Data) myDataFramework;
    TDF_Label Root            = myDataFramework->Root();       // node "0"
    TDF_Label boxLabel        = TDF_TagSource::NewChild(Root); // "0:1"
    TDF_Label selectionLabel  = TDF_TagSource::NewChild(Root); // "0:2"
    TDF_Label filletLabel     = TDF_TagSource::NewChild(Root); // "0:3"
    TDF_Label updatedBoxLabel = TDF_TagSource::NewChild(Root); // "0:4"
    TDF_Label updtFilletLabel = TDF_TagSource::NewChild(Root); // "0:5"

    // You'll need to add each face of 'myBox' to the DataFramework as a 'generated' shape
    // NOTE: here, AddGeneratedFaces method is not defined. It seems like it will store on
    // boxLabel the Faces passed in to it, and may add the text 'Generated Faces' to it as
    // well. Faces would likely be something like e.g. std::vector<TopoDS_Face> Faces =
    // {myBox.Top(), myBox.Bottom() etc...}
    AddGeneratedFaces(boxLabel, "Generated Faces", Faces)
    TNaming_Selector mySelector(selectionLabel);

    // select the 3rd Edge
    TopTools_IndexedMapOfShape Edges;
    TopExp::MapShapes(myBox.Shape(), TopAbs_EDGE, Edges);
    TopoDS_Edge Edge3 = TopoDS::Edge(Edges.FindKey(3));
    // Note, this means "Select Edge3, which can be found in myBox.Shape()"
    mySelector.Select(Edge3, myBox.Shape());

    // Fillet that edge and add the filleted shape to the DataFramework
    BRepFilletAPI_MakeFillet mkFillet(myBox.Shape());
    mkFillet.Add(1., 1., Edge3);
    mkFillet.Build();
    // Note: 'ModifiedData' will need to contain all of the Modified, Deleted, and
    // Generated Faces. For this simple Fillet operation, there will be four modified
    // Faces on the Box, and one generated Face which is the fillet face itself
    AddModifiedShape(filletLabel, "Filleted Shape, ModifiedData);

    // Now make the box taller
    BRepPrimAPI_MakeBox myBox2(10., 10., 15.);
    // Note: Faces2 will contain all faces of the box except the Bottom face, which will
    // not change if the height is adjusted. AddModifiedFaces will need to call
    // `TNaming_Builder::Modify` for each face, with e.g. `oldShape = myBox.Shape().Top()` and
    // `newShape = myBox2.Shape().Top()` etc...
    AddModifiedFaces(updatedBoxLabel, "Modifies Faces", Faces2);

    // Finally, we want to re-fillet the updated box, and make sure that we're filleting
    // the same edge.
    TDF_LabelMap myMap;
    mySelector.Solve(myMap);
    TopoDS_Edge targetEdge = TopoDS::Edge(mySelector.NamedShape()->Get()); 
    BRepFilletAPI_MakeFillet mkFillet2(myBox2.Shape());
    mkFillet2.Add(1., 1., targetEdge);
    mkFillet2.Build();
    AddModifiedShape(updtFilletLabel, "Update Fillet", ModifiedData2);

So, this example won't run as-is but should provide a basic understanding of how
`TNaming_Selector` works. Here are some key points to know about `TNaming_Selector`

1. `TNaming_Selector::Select` returns a `bool` that tells you whether or not the Selection
   was succesful
2. `TNaming_Selector::Solve` must be called in order to update the reference to the
   selected entities. In our case, after our box was changed but before
   `TNaming_Selector::Solve` was called, the stored Edge was still on the original
   10x10x10 box
3. `TNaming_Selector::Solve` takes as an argument a `TDF_LabelMap` that is a list of
   labels. This is, I think, a way to specify to the Solve algorithm a sub-set of the
   entire `TDF_Data` tree that should be considered for solving the topological naming. If
   a blank `TDF_LabelMap` is specified, then the whole tree is used for the algorithm.
4. It is extremely important to the Solve algorithm that the rules outlined
   [here](http://www.opencascade.com/doc/occt-7.0.0/overview/html/occt_user_guides__ocaf.html#occt_ocaf_5_6_1)
   in the OCC guides are adhered to.
5. TNaming_Builder appears to have a Select method. This method, however, is not ever used
   in the occ example. Instead, TNaming_Selector::Select is used. Not sure if they are
   interchangeable, though I have a feeling they're not.

# How to use TNaming to resolve Topological Naming

Well, in some of the early tests, it appears that simply retaining a reference to a
particular TDF_Label that holds a TNaming_NamedShape is sufficient to maintain a constant,
non-volatile reference to the TopoDS_Shape that the TNaming_NamedShape is holding. All it
takes is a simple `myNamedShape->Get()` to grab the TopoDS_Shape.

**UPDATE:** I do not think that the previous paragraph actually holds up. I'm not sure
what I was seeing in my earlier tests, but I do believe `TNaming_Selector` _must_ be used
in order to ensure that a constant reference to a given topological entity in maintained.
I'm leaving the paragraph in the document for now until I can go back and do some more
tests to shed some light on what was going on in those earlier tests.

## Modeling History
In order for the occ TNaming framework to do it's thing, it requires that the Data
Framework contain certain pieces of information regarding the 'Evolution' of the
TopoDS_Shape under consideration. The occ documentation provides a nice table
[here](http://www.opencascade.com/doc/occt-7.0.0/overview/html/occt_user_guides__ocaf.html#occt_ocaf_5_6_1),
but in short, if a modeling operation results in:

- a Solid or Closed Shell, all the Faces must be recorded
- a Face or Open Shell, all the Faces and open Edges must be recorded
- There are other rules and a few exceptions, check the occ documentation

What do I mean by 'must be recorded'? Well, that's what the `TNaming_NamedShape` is all
about. You use a `TNaming_Builder` to record which sub-shapes are changed, and how they
are changed. Again, I'll refer to [a section in the occ
documentation](http://www.opencascade.com/doc/occt-7.0.0/overview/html/occt_user_guides__ocaf.html#occt_ocaf_5_6_1)
for a more detailed (though far from exhaustive, their documentation is not great)
explanation, but briefly, a `TopoDS_Shape` can have one of five 'Evolution's:

- PRIMITIVE: new topology, usually created using BRepPrimAPI
- GENERATED: new topology created from another topology, such as a Face created from an
  Edge
- MODIFY: new topology is a modification of another topology, i.e. oldShape=origBox,
  newShape=boxWithAHole
- DELETE: some piece of topology was deleted, i.e. a Face on a Box is Deleted to create an
  open shell
- SELECTED: see the previous section, this topology is specially marked as something to
  maintain a constant reference to.

Each of these Evolutions has a "new_shape" and "old_shape", and the occ documentation does
a decent job explaining what each of these are for each Evolution.

So, any time you cut a hole out of a solid, you need to record what happened to each face
in the resulting solid. I think in this case, all Faced are MODIFY.

Something the occ documentation mentions briefly, but which appears to be important, is
that aside from following these 'rules', the resultant shape itself must also be stored
with an Evolution. So, for the case of cutting a hole out of a solid, you need to store
the boxWithAHole as MODIFY with origBox as the old_shape, as well as iterating through all
the Faces and storing them appropriately.

How do you store the information? Well, the occ documentation doesn't really specify, and
I'm not sure if it really matters, but I think what makes sense is to follow the example
from the occ code. In other words:

1. Each modeling operation should create a new node under Root in the Data Framework
2. Each of these nodes will contain the result of the modeling operation with the
   appropriate Evoluion
3. Each of these nodes will contain a list of sub-nodes that will follow the 'rules'
   outlined above and in the occ documentation, and will contain the proper Evolution for
   each of the appropriate sub-shapes
4. For some reason, when `Select`ing Edges for a Fillet operation, that Selection gets its
   own Node.
