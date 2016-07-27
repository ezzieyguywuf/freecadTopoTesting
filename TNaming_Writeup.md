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
*   along with this program.  If not, see [http://www.gnu.org/licenses/].        *
******************************************************************************** */
</pre>
# What is TNaming?

TNaming is an opencascade (henceforth referred to as occ) 'Package'. It provides a set of
tools for implementing a Topological Naming structure using the TDF_Data class. What is
TDF_Data? It is the class used in the TDF 'Package' for storing and manipulating a Data
Framework (not sure what the T stands for). The occ Documentation does a good job
explaining the Data Framework and how it works, but here's a few highlights.

## occ Data Framework
Note: Please refer to the occ Documentation for a more detailed explanation.
[Here](http://www.opencascade.com/doc/occt-7.0.0/overview/html/occt_user_guides__ocaf.html#occt_ocaf_3)
is a direct link. It is under the 'OCAF' section of the Documentation, but fear not
because the Data Framework can be used outside the context of the OCAF thing.

So, the way that occ's TDF package manages Data in it's Data Framework is by creating
nodes in a tree, all stored in an instance of TDF_Data. Each node has a unique Tag used
for identifying it in the Data Framework. Each of these Tag's points to a discrete
TDF_Label in the Data Framework. The TDF_Label, in turn, contains zero or more
TDF_Attribute's, which themselves contain actual data that you may be interested in.

Why the distinction between Tag and TDF_Label and TDF_Attribute? Well, I can't speak to
the original intent of the occ devs, but it makes sense to me that you'd want a different
Tag and TDF_Label. Since the TDF_Label can, itself, contain many TDF_Attribute's that can
themselves contain a lot of data, it could be quite expensive to store an instance of each
TDF_Label that you create and care about. In fact, it would be wasteful because your
TDF_Data instance is already going to have a copy of each TDF_Label, so you'd be doubling
your memory footprint for no reason. Instead, store a value of the Tag itself, which is
nothing more than an integer, and then use TDF_Tool::TagList (I think! I haven't tested
this yet) to retrieve the TDF_Label.

So what is an TDF_Attribute? Like I said earlier, it is the actual 'meat' of the  occ Data
Framework. The Tag is just an Address to a TDF_Label, which is just a construct that holds
attributes. The TDF_Attribute is a class that contains whatever information it is that you
care about. TNaming_NamedShape is a TDF_Attribute that is described in the TNaming
Package. Make sense? Lol, there appear to be many other TNaming_Attribute's that occ has
defined, I guess expecting that they are common things that people will want to store. For
a full list, check
[here](http://www.opencascade.com/doc/occt-6.9.1/refman/html/class_t_d_f___attribute.html).

One more thing and TDF_Attribute's. A TDF_Label can only contain **one** of a given
TDF_Attribute. I don't really know why, I just know it's true. For this reason, every
TDF_Label that occ has defined has it's own unique GUID, which can be accessed via the
_class method_ `GetID()`. What I mean by 'class method' is that you do **_not_** do the
following:

    TNaming_NamedShape myNamedShape();
    Standard_GUID myGUID = myNamedShape.GetID() // WRONG! Don't do this!

Instead, what you want to do is simply

    Standard_GUID myGUID = TNaming_NamedShape::GetID() // Proper way to access class method

This GUID thing is important b/c it is the mechanism by which you grab a desired
TDF_Attribute from a TDF_Label. Consider it the equivalent of a Tag - it is the address by
which we access a TDF_Attribute in a TDF_Label.

Typical usage of TDF_Label, as well as retrieving an Attribute from a label:

    TDF_Label myLabel = fatherLabel.FindChild(1); // created label '1' if it didn't exist
    TDF_Label testLabel = fatherLabel.FindChild(1); // retrieved label '1'

    // I haven't used this yet, but it seems to keep track of unique label numbers for you
    TDF_Label otherLabel = TDF_TagSource::NewChild(fatherLabel); // probably '2'

    Handle(TNaming_NamedShape)  myNamedShape;
    // this stores in myNamedShape a pointer to the TNaming_NamedSHape in someLabel
    someLabel.FindAttribute(TNaming_NamedShape::GetID(), myNamedShape);

## TNaming_NamedShape

So, now that we know what a Tag, a TDF_Label, and a TDF_Attribute are, let's talk about
TNaming_NamedShape. TNaming_NamedShape inherits from TDF_Attribute: in other words, it is
a TDF_Attribute. It is actually a 'helper TDF_Attribute', if you will, that writes and
reads information from TNaming_UsedShapes. I don't know a ton about TNaming_UsedShapes
right now, but suffice to say that it:

1. Is a TDF_Attribute
2. Only exists on the root node of the the TDF_Data tree structure
3. Is created for you whenever you add a TNaming_NamedShape to your TDF_Data tree
4. Should never really need to be accessed directly (I think)
5. Most importantly, it is the grand-daddy of the whole Topological Naming algorithm and
mechanism that occ has implemented. It stores a list of all the TopoDS_Shape's that are
used in the evolution of the Shape.

The only method of the TNaming_NamedShape class I've found myself using is
`myNamedShape.Get()`, or more often `myNamedShape->Get()` as I'm typically dealing with a
pointer (or `Handle` in occ terms) to the TNaming_NamedShape. This method returns the 
TopoDS_Shape which is stored inside the TNaming_NamedShape (or rather, the TopoDS_Shape
that the TNaming_NamedShape points to in the TNaming_UsedShapes attribute, if you want to
be technical).

You may be wondering "if the TNaming_NamedShape attribute is so important, how come we're
only using the Get method? How do we add information to the TNaming_NamedShape attribute?"
Good question! This is actually done using the TNaming_Builder class. As the name implies,
the TNaming_Builder class helps us to 'build' TNaming_NamedShape attributes. Again, I
won't deign to speak for the intent of the occ devs, but I imagine the reason for the
separate TNaming_Builder is to separate the entries into the TNaming_UsedShape structure
from the creation of those entries. For example, if you look at the [occ
documentation](http://www.opencascade.com/doc/occt-7.0.0/overview/html/occt_user_guides__ocaf.html#occt_ocaf_5_3),
you'll notice that TNaming_NamedShape tracks the "evolution" of it's TopoDS_Shape in 5
different ways. TNaming_Builder makes it easier for us to insert these different
evolutions into the TNaming_NamedShape attribute, because there's some funky
behind-the-scenes stuff going on with some sort of TNaming_Node construct.

Things to know about TNaming_Builder:

1. Always use a TNaming_Builder to add TopoDS_Shape evolution information to a
TNaming_NamedShape
2. TNaming_Builder takes as an argument to its constructor the TDF_Label on which to
create the TNaming_NamedShape attribute.
  a. If no TNaming_UsedShapes exists in the root of the TDF_Data tree, it creates it
  b. If no TNaming_NamedShape exists at this TDF_Label, it creates one
  c. If a TNaming_NamedShape already exists at this TDF_Label, it creates some sort of
  backup, then clears all the data so that it can write the new data. I'm not certain, but
  I think this backup mechanism may be used by the TNaming_UsedShape thing and by the
  Topological Naming algorithm to figure things out.
3. Only use one TNaming_Builder per evolution type. In fact, I think it's safest to just
create a new TNaming_Builder any time you're dealing with a new operation.

Here's a typical use-case:

    TNaming_Builder myBuilder(myLabel);
    myBuilder.Generated( anEdge, aFace); // aFace was generated from anEdge

    TNaming_Builder myBuilder2(myLabel2);
    myBuilder2.Modify( origShape, newShape ); // origShape was somehowe modified (cut, fuse, etc...) into newShape

    TNaming_Builder myBuilder3(myLabel3);
    myBuilder3.Select( anEdge, aShape); // anEdge is an edge within the contex of aShape.

# How to use TNaming to resolve Topological Naming

Well, in some of the early tests, it appears that simply retaining a reference to a
particular TDF_Label that holds a TNaming_NamedShape is sufficient to maintain a constant,
non-volatile reference to the TopoDS_Shape that the TNaming_NamedShape is holding. All it
takes is a simple `myNamedShape->Get()` to grab the TopoDS_Shape.

## Selector caveat
Upon further reading, though, the occ documentation goes to great lengths to describe the
TNaming_Selector mechanism, and seems to imply that it is _this_ that provides the robust
references we are looking for. More extensive testing should be done to verify this, but
for now let's accept the occ documentation at face value.

That being the case, in order to maintain a constant reference to a topological entity,
that entity must be 'Selected'. This is done by using
TNaming_Selector.Select(shapeToSelect, contextShape). This stores in the label - which is
passed to the TNaming_Selector constructor - a TNaming_NamedShape with the TNaming_SELECTED
Evolution. This is the TNaming_NamedShape you'll want to refer back to if you need that
Shape back.

Some things to know about Selecting things:

1. TNaming_Builder appears to have a Select method. This method, however, is not ever used
in the occ example. Instead, TNaming_Selector::Select is used. Not sure if they are
interchangeable
2. Apparently, you want to call TNaming_Selector::Solve() any time the contextShape
changes in order to re-solve the Selected Shape.
3. I still don't fully understand this Selection stuff, so stay tuned, and travel with
caution.

## Modeling History
In order for the occ TNaming framework to do it's thing, it requires that the Data
Framework contain certain pieces of information regarding the 'Evolution' of the
TopoDS_Shape under consideration. The occ documentation provides a nice table
[here](http://www.opencascade.com/doc/occt-7.0.0/overview/html/occt_user_guides__ocaf.html#occt_ocaf_5_6_1),
but in short, if a modeling operation results in:

- a Solid or Closed Shell, all the Faces must be recorded
- a Face or Open Shell, all the Faces and open Edges must be recorded
- There are other rules and a few exceptions, check the occ documentation

What do I mean by 'must be recorded'? Well, that's what the TNaming_NamedShape is all
about. You use a TNaming_Builder to record which sub-shapes are changed, and how they are
changed. Again, I'll refer to [a section in the occ
documentation](http://www.opencascade.com/doc/occt-7.0.0/overview/html/occt_user_guides__ocaf.html#occt_ocaf_5_6_1)
for a more detailed (though far from exhaustive, their documentation is not great)
explanation, but briefly, a TopoDS_Shape can have one of five 'Evolution's:

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
in the resulting solid. I think in tis case, all Faced are MODIFY.

Something the occ documentation mentions briefly, but which appears to be important, is
that aside from following these 'rules', the resultant shape itself must also be stored
with an Evolution. So, for the case of cutting a hole out of a solid, you need to store
the boxWithAHole as MODIFY with origBox as the old_shape, as well as iterating through all
the Faces and storing them appropriately.

How do you store the information? Well, the occ documentation doesn't really specify, and
I'm not sure if it really matter, but I think what makes sense is to follow the example
from the occ code. In other words:

1. Each modeling operation should create a new node under Root in the Data Framework
2. Each of these nodes will contain the result of the modeling operation with the
   appropriate Evoluion
3. Each of these nodes will contain a list of sub-nodes that will follow the 'rules'
   outlined above and in the occ documentation, and will contain the proper Evolution for
   each of the appropriate sub-shapes
4. For some reason, when Select'ing Edges for a Fillet operation, that Selection gets its
   own Node.
