#include <iostream>

#include <TDF_Data.hxx>
#include <TDF_Label.hxx>
#include <TDF_LabelMap.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_MapIteratorOfLabelMap.hxx>

#include <TNaming_NamedShape.hxx>
#include <TNaming_Selector.hxx>
#include <TNaming_Tool.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming.hxx>

#include <Geom_Plane.hxx>
#include <Geom_TrimmedCurve.hxx>

#include <GC_MakeSegment.hxx>

#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>

#include <TopLoc_Location.hxx>

#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>

#include <TopOpeBRepBuild_HBuilder.hxx>

#include <BRep_Tool.hxx>

#include <BRepTools.hxx>

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakePrism.hxx>

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>

#include <BRepAlgo.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgo_Cut.hxx>

#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Vec.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>

void printShapeInfos(TopoDS_Shape shape1, TopoDS_Shape shape2, TopoDS_Shape shape3);
void writeShapeInfos(TopoDS_Shape shape1, TopoDS_Shape shape2, TopoDS_Shape shape3);
void printShapeInfo(TopoDS_Shape shape);
void writeShapeInfo(TopoDS_Shape shape, std::ofstream&);
void printDumpFile(TopoDS_Shape shape);
void writeDumpFiles(TopoDS_Shape orig, TopoDS_Shape cut, TopoDS_Shape newBox);

void printShapeInfos(TopoDS_Shape shape1, TopoDS_Shape shape2, TopoDS_Shape shape3){
    std::cout << "orig box" << std::endl;
    printShapeInfo(shape1);
    std::cout << "cut box" << std::endl;
    printShapeInfo(shape2);
    std::cout << "new box" << std::endl;
    printShapeInfo(shape3);
}

void writeShapeInfos(TopoDS_Shape shape1, TopoDS_Shape shape2, TopoDS_Shape shape3){
    std::ofstream myOutFile;
    // open output files
    myOutFile.open("shapeInfos.txt");

    myOutFile << "orig box" << std::endl;
    writeShapeInfo(shape1, myOutFile);
    myOutFile << "cut box" << std::endl;
    writeShapeInfo(shape2, myOutFile);
    myOutFile << "new box" << std::endl;
    writeShapeInfo(shape3, myOutFile);
    myOutFile.close();
}

void printShapeInfo(TopoDS_Shape shape){
    TopTools_IndexedMapOfShape mapOfShapes;
    TopExp::MapShapes(shape, TopAbs_FACE, mapOfShapes);
    for (int i = 1; i <= mapOfShapes.Extent(); i++){
        TopoDS_Face aFace = TopoDS::Face(mapOfShapes.FindKey(i));
        Handle(Geom_Surface) aSurface = BRep_Tool::Surface(aFace);
        if (aSurface->DynamicType() == STANDARD_TYPE(Geom_Plane)){
            Handle(Geom_Plane) aPlane = Handle(Geom_Plane)::DownCast(aSurface);
            gp_Pnt aPoint = aPlane->Location();
            gp_Ax1 anAxis = aPlane->Axis();
            gp_Dir aDir   = anAxis.Direction();
            Standard_Real locx, locy, locz, dirx, diry, dirz;
            locx = aPoint.X();
            locy = aPoint.Y();
            locz = aPoint.Z();
            dirx = aDir.X();
            diry = aDir.Y();
            dirz = aDir.Z();
            std::cout << "For i= " << i << " ";
            std::cout << "loc = (" << locx << ", " << locy << ", " << locz << ") , ";
            std::cout << "dir = (" << dirx << ", " << diry << ", " << dirz << ") " << std::endl;
        }
    }
}

void writeShapeInfo(TopoDS_Shape shape, std::ofstream& myOutFile){
    TopTools_IndexedMapOfShape mapOfShapes;
    TopExp::MapShapes(shape, TopAbs_FACE, mapOfShapes);

    for (int i = 1; i <= mapOfShapes.Extent(); i++){
        TopoDS_Face aFace = TopoDS::Face(mapOfShapes.FindKey(i));
        Handle(Geom_Surface) aSurface = BRep_Tool::Surface(aFace);
        if (aSurface->DynamicType() == STANDARD_TYPE(Geom_Plane)){
            Handle(Geom_Plane) aPlane = Handle(Geom_Plane)::DownCast(aSurface);
            gp_Pnt aPoint = aPlane->Location();
            gp_Ax1 anAxis = aPlane->Axis();
            gp_Dir aDir   = anAxis.Direction();
            Standard_Real locx, locy, locz, dirx, diry, dirz;
            locx = aPoint.X();
            locy = aPoint.Y();
            locz = aPoint.Z();
            dirx = aDir.X();
            diry = aDir.Y();
            dirz = aDir.Z();
            myOutFile << "For i= " << i << " ";
            myOutFile << "loc = (" << locx << ", " << locy << ", " << locz << ") , ";
            myOutFile << "dir = (" << dirx << ", " << diry << ", " << dirz << ") " << std::endl;
        }
    }
}

void printDumpFile(TopoDS_Shape shape){
    BRepTools::Dump(shape, std::cout);
}

void writeDumpFiles(TopoDS_Shape orig, TopoDS_Shape cut, TopoDS_Shape newBox){
    std::ofstream myBoxFile, cutBoxFile, newBoxFile;

    // open output files
    myBoxFile.open("orig_box.txt");
    cutBoxFile.open("cut_box.txt");
    newBoxFile.open("new_box.txt");

    BRepTools::Dump(orig, myBoxFile);
    BRepTools::Dump(cut, cutBoxFile);
    BRepTools::Dump(newBox, newBoxFile);

    // close output files
    myBoxFile.close();
    cutBoxFile.close();
    newBoxFile.close();
}

void cutBoxAndShowThings(TopoDS_Shape origBox){
    TopoDS_Shape cutBox, newBox;
    gp_Pnt cut_loc;
    gp_Dir cut_dir;
    gp_Ax2 cut_ax2;
    // create box to cut out and write it out.
    cut_loc = gp_Pnt(7, 5, 30);
    cut_dir = gp_Dir(0, 0, -1);
    cut_ax2 = gp_Ax2(cut_loc, cut_dir);
    cutBox = BRepPrimAPI_MakeBox(cut_ax2, 5., 5., 5.).Shape();

    // create new box by cutting out the second box from the first
    BRepAlgoAPI_Cut builder(origBox, cutBox);
    newBox = builder.Shape();

    // print out Dump to files
    writeDumpFiles(origBox, cutBox, newBox);
    //printDumpFile(origBox);
    //printDumpFile(cutBox);
    //printDumpFile(newBox);

    writeShapeInfos(origBox, cutBox, newBox);
    //printShapeInfos(origBox, cutBox, newBox);
}

void runCase1(){
    // Variable defs
    TopTools_ListIteratorOfListOfShape iterator;
    TopoDS_Shape myBox;

    // say hello
    std::cout << "Hello, Running Case 1" << std::endl;

    // make base box
    BRepPrimAPI_MakeBox mkBox(10., 20., 30.);
    myBox = mkBox.Shape();

    cutBoxAndShowThings(myBox);
}

void runCase2(){
    // Variable defs
    TopTools_ListIteratorOfListOfShape iterator;
    gp_Pnt v1, v2, v3, v4;
    TopoDS_Shape myBox;
    // say hello
    std::cout << "Hello, Running Case 2" << std::endl;

    // define points for outline
    v1 = gp_Pnt(0, 0, 0);
    v2 = gp_Pnt(0, 20, 0);
    v3 = gp_Pnt(10, 20, 0);
    v4 = gp_Pnt(10, 0 ,0);

    // define line segments of outline
    Handle(Geom_TrimmedCurve) segment1 = GC_MakeSegment(v1, v2);
    Handle(Geom_TrimmedCurve) segment2 = GC_MakeSegment(v2, v3);
    Handle(Geom_TrimmedCurve) segment3 = GC_MakeSegment(v3, v4);
    Handle(Geom_TrimmedCurve) segment4 = GC_MakeSegment(v4, v1);

    // define the topological edges based on these segments
    TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(segment1);
    TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(segment2);
    TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(segment3);
    TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(segment4);

    // define the wire which is our outline
    BRepBuilderAPI_MakeWire mkWire;
    mkWire.Add(edge1);
    mkWire.Add(edge2);
    mkWire.Add(edge3);
    mkWire.Add(edge4);
    TopoDS_Wire myOutline = mkWire.Wire();

    // define a face based on our wire, which is what we'll extrude
    TopoDS_Face myFace = BRepBuilderAPI_MakeFace(myOutline);

    // Finally, extrude the face
    gp_Vec extrudeVector(0, 0, 30); // Specifies direction AND length of extrude
    myBox = BRepPrimAPI_MakePrism(myFace, extrudeVector);

    cutBoxAndShowThings(myBox);
}

void runCase3(){
    // This is for the Data Framework
    Handle(TDF_Data) DF = new TDF_Data();
    TDF_Label aLabel = DF->Root();

    // Create our base box
    BRepPrimAPI_MakeBox myBox(10., 20., 30.);

    // Load the faces of the box into the Data Framework. This adds nodes to the
    // DataFramework tree, I believe. Note that myBoxLabel is added to the Root, while all
    // the rest are added underneath myBoxLabel, so they're at level 3. NOTE: it is not
    // necesarry to pass the second argument to FindChild, but I like to do so in order to
    // make it more explicit that I am asking it to create this Label if it doesn't exist.
    const Standard_Boolean createIfNotExists = Standard_True;
    TDF_Label myBoxLabel      = aLabel.FindChild(1, createIfNotExists); // First Label in Root
    TDF_Label myBoxTopLabel   = myBoxLabel.FindChild(1, createIfNotExists); // All these are under the first Label
    TDF_Label myBoxBotLabel   = myBoxLabel.FindChild(2, createIfNotExists);
    TDF_Label myBoxRightLabel = myBoxLabel.FindChild(3, createIfNotExists);
    TDF_Label myBoxLeftLabel  = myBoxLabel.FindChild(4, createIfNotExists);
    TDF_Label myBoxFrontLabel = myBoxLabel.FindChild(5, createIfNotExists);
    TDF_Label myBoxBackLabel  = myBoxLabel.FindChild(6, createIfNotExists);

    // TNaming_Builder takes a label as it's constructor. It allows us to add 'attributes'
    // to the Label (see the OCAF documentation about Data Framework for more info). These
    // attributes, I believe, help us track changes to a Topological structure. This is
    // how Generated, Modified, and Deleted attributes are kept track of. Oh, and
    // apparently "One evolution type per builder must be used"
    TNaming_Builder myBoxBuilder(myBoxLabel);
    // Add the 'Generated' attribute to the myBox TopoDS_Shape. The myBoxLabel Label is
    // where we are storing attributes for the whole myBox TopoDS_Shape. Underneath that
    // label, we created other labels for each of the Faces, where we can track changes to
    // those Topological structures.
    myBoxBuilder.Generated(myBox.Shape());

    // create a Builder for each Face and add the 'Generated' attribute to the Data
    // Framework
    TNaming_Builder topBuilder(myBoxTopLabel);
    // Note, this is sort of cheating, for other Topo structures we'll have to do this on our own
    TopoDS_Face myBoxTopFace = myBox.TopFace();
    topBuilder.Generated(myBoxTopFace);

    // And, do the rest of the faces...
    TNaming_Builder botBuilder(myBoxBotLabel);
    TopoDS_Face myBoxBotFace = myBox.BottomFace();
    botBuilder.Generated(myBoxBotFace);

    TNaming_Builder rightBuilder(myBoxRightLabel);
    TopoDS_Face myBoxRightFace = myBox.RightFace();
    rightBuilder.Generated(myBoxRightFace);

    TNaming_Builder leftBuilder(myBoxLeftLabel);
    TopoDS_Face myBoxLeftFace = myBox.LeftFace();
    leftBuilder.Generated(myBoxLeftFace);

    TNaming_Builder frontBuilder(myBoxFrontLabel);
    TopoDS_Face myBoxFrontFace = myBox.FrontFace();
    frontBuilder.Generated(myBoxFrontFace);

    TNaming_Builder backBuilder(myBoxBackLabel);
    TopoDS_Face myBoxBackFace = myBox.BackFace();
    backBuilder.Generated(myBoxBackFace);

    // Create second box, which will be Cut from the first
    gp_Pnt cut_loc = gp_Pnt(7, 5, 30); // location for axis of cutBox
    gp_Dir cut_dir = gp_Dir(0, 0, -1); // direction for axis of cutBox
    gp_Ax2 cut_ax2 = gp_Ax2(cut_loc, cut_dir); // axis at cut_loc pointing down, so into our myBox
    BRepPrimAPI_MakeBox cutBox(cut_ax2, 5., 5., 5.);

    // It seems that even though cutBox is not a Fused part of our TopoDS_Shape, since it
    // is still a 'part' of it by means of being Cut out of it, we must store information
    // about it in our Data Framework. This is the equivalent of what we did for myBox,
    // and I think finding a way to write a generic function that accomplishes this is in
    // order, as doing it manually like this is not sustainable for the greater FreeCAD
    // project

    // number here is arbitrary, though we do have to keep track of our numbers somehow.
    // i.e. we used 1 for myBoxLabel, so we can't reuse it, and I have to remember that 1
    // correspondes to myBoxLabel, not cutBox. Seems like a job for a custom class or
    // something. hrm...
    TDF_Label cutBoxLabel      = aLabel.FindChild(2, createIfNotExists);
    TDF_Label cutboxTopLabel   = cutBoxLabel.FindChild(1, createIfNotExists); // same thing about numbering as above, but for children
    TDF_Label cutBoxTopLabel   = cutBoxLabel.FindChild(1, createIfNotExists); // All these are under the first Label
    TDF_Label cutBoxBotLabel   = cutBoxLabel.FindChild(2, createIfNotExists);
    TDF_Label cutBoxRightLabel = cutBoxLabel.FindChild(3, createIfNotExists);
    TDF_Label cutBoxLeftLabel  = cutBoxLabel.FindChild(4, createIfNotExists);
    TDF_Label cutBoxFrontLabel = cutBoxLabel.FindChild(5, createIfNotExists);
    TDF_Label cutBoxBackLabel  = cutBoxLabel.FindChild(6, createIfNotExists);

    TNaming_Builder cutBoxBuilder(cutBoxLabel);
    cutBoxBuilder.Generated(cutBox.Shape());

    TNaming_Builder topCutBuilder(cutBoxTopLabel);
    TopoDS_Face cutBoxTopFace = cutBox.TopFace();
    topCutBuilder.Generated(cutBoxTopFace);

    TNaming_Builder botCutBuilder(cutBoxBotLabel);
    TopoDS_Face cutBoxBotFace = cutBox.BottomFace();
    botCutBuilder.Generated(cutBoxBotFace);

    TNaming_Builder rightCutBuilder(cutBoxRightLabel);
    TopoDS_Face cutBoxRightFace = cutBox.RightFace();
    rightCutBuilder.Generated(cutBoxRightFace);

    TNaming_Builder leftCutBuilder(cutBoxLeftLabel);
    TopoDS_Face cutBoxLeftFace = cutBox.LeftFace();
    leftCutBuilder.Generated(cutBoxLeftFace);

    TNaming_Builder frontCutBuilder(cutBoxFrontLabel);
    TopoDS_Face cutBoxFrontFace = cutBox.FrontFace();
    frontCutBuilder.Generated(cutBoxFrontFace);

    TNaming_Builder backCutBuilder(cutBoxBackLabel);
    TopoDS_Face cutBoxBackFace = cutBox.BackFace();
    backCutBuilder.Generated(cutBoxBackFace);

    // OK, here's the hard/fun part. Let's cut cutBox from myBox and do all the
    // appropriate things to the DataFramework

    // First, we create anoter label in our DataFramework under Root. In other words, our
    // first label was for myBox, our second was for cutBox (both TopoDS_Shapes) and our
    // third label is for the resultant box of the Cut operation. we'll call it myBoxCut
    TDF_Label myBoxCutLabel = aLabel.FindChild(3, createIfNotExists);

    // Note: this is an example of how to recover an object from a particular node in our
    // Data Framework. This is key, as this is where opencascade takes care of the Robust
    // Reference thing for us. You'll notice that in this particular instance we don't
    // actually need to use this mechanism, as I still have a reference to myBox handy and
    // it hasn't been modified, but I'm doing this here for educational purposes.

    Handle(TNaming_NamedShape) namedShapeObject; // store the retrieved object here
    // This part of the occ example confounds me. How in the world could this CLASS METHOD
    // GetID know anything about an instance of TNaming_NamedShape enough to return to me
    // the GUID that I'm looking for? The occ documentation under "OCAF Shape
    // Attributes-->Using Naming Resources" suggests that there is also a method that each
    // object "of an attribute class" has that will return the GUID. this makes more sense
    // to me..
    myBoxLabel.FindAttribute(TNaming_NamedShape::GetID(), namedShapeObject);
    TopoDS_Shape myRecoveredBox = namedShapeObject->Get();

    // let's check if it's the same box!
    Standard_Boolean checkBox = myBox.Shape().IsEqual(myRecoveredBox);
    std::string output;
    if (checkBox)
        output = "True";
    else
        output = "False";
    std::cout << "myBox == myRecovered Box == " << output << std::endl;
    // output "myBox == myRecoveredBox == True" woah! it works! So, this should also work
    // for faces and stuff, right....?

    // Ok, I think I got a bit ahead of myself. I think it's this TNaming_Selector thing
    // that is key - we create a new TNaming_Selector for each TNaming_Label that we need
    // a constant reference too, and then it gives us a consistent, er, Name for each
    // TopoDS_Shape?

    TDF_Label resultCutLabel = myBoxCutLabel.FindChild(1); // child 1 of myBoxCutLabel, which is the third node in Root right now
    TNaming_Selector myBoxCutSelector(resultCutLabel);
    Handle(TNaming_NamedShape) cutBoxNamedShape;
    cutBoxLabel.FindAttribute(TNaming_NamedShape::GetID(), cutBoxNamedShape);

    // like above, this is not necessary b/c I still have a reference to cutBox from
    // earlier, but for completeness and b/c IRL (in real life) this will be done in a
    // separate function, I'm putting it in.
    const TopoDS_Shape& cutBoxRecovered = cutBoxNamedShape->Get();
    // The two arguments here are 'selection' and 'context', respectively. This means
    // "store in NamedShape (which is an instance attribute of this TNaming_Selector
    // instance) the name for the TopoDS_Shape 'selection' which is found with the
    // TopoDS_Shape 'context'. The 'context' thing probably makes more sense when
    // searching for a Face within a Shape, or an Edge within a Face.
    myBoxCutSelector.Select(cutBoxRecovered, cutBoxRecovered);

    // Finally, grab the TopoDS_Shape that resulted from the 'Select' operation. I could
    // do this in two steps, i.e. `TNaming_NamedShape ns = myBoxCutSelector.NamedShape()`
    // then `TopoDS_Shape& myShape = ns->Get()` but I won't. Doing it in two steps,
    // though, does highlight how the 'Select' method works 'behind the scenes' and stores
    // the output TNaming_NamedShape inside of itself, and then you have to go back and
    // grab it.
    //
    // So, why is this step necessary, if we alread have cutBoxRecovered? I think it's
    // because of the following: cutBoxRecovered is very 'young' in our Data Framework
    // tree. It was the orignial Box that was cut from the original grand-daddy Box myBox.
    // Other stuff may have happened since then. In this simple example, nothing else has
    // happened, but realistically we could be WAY down the Data Framework tree by now,
    // after Fillet operations and Fuse operations oter Boolean operations. So, although
    // we have a reference to the very 'young' cutBox, we don't really have a reference to
    // that box within the current context of our TopoDS_Shape. So, this next operation
    // get's us the same cutBox, but relative to the current state of the TopoDS_Shape.
    //
    // I think. This is all pure speculation. But I think it's right!
    const TopoDS_Shape& cutBoxSelected = myBoxCutSelector.NamedShape()->Get();

    // finally, now that we've retrieved both the box to cut and the box that we're going
    // to cut from it, we can perform the Cut operation and then store the necessary stuff
    // in our Data Framework

    BRepAlgo_Cut mkCut(myRecoveredBox, cutBoxSelected);
    TopoDS_Shape resultBox = mkCut.Shape();
    if (!mkCut.IsDone()) {
        cout << "CUT: Algorithm failed" << endl;
        return; 
    }
    else {
        // Alright, take a deep breath, b/c this stuff goes deep

        // First, it seems we must use BRepAlgo::IsValid to check if the faces generated
        // by the mkCut operation are Valid. *shrug* the occ example does it, so I do it
        TopTools_ListOfShape listBoxes;
        listBoxes.Append(myRecoveredBox);
        listBoxes.Append(cutBoxSelected);

        if (!BRepAlgo::IsValid(listBoxes, resultBox, Standard_True, Standard_False)) {
            cout << "CUT: Result is not valid" << endl;
            return;
        }
        else {
            // We're going to add data to the Data Framework. This data will be added to
            // the third node under the Root: we called it myBoxCutLabel. The data we are
            // adding corresponds to the result box from the Cut operation, and will store
            // the Modified, Deleted, and other relevant data that the Data Framework
            // needs.

            TDF_Label modifiedFacesLabel     = myBoxCutLabel.FindChild(2); // remember, 1 was for the result of the Cut operation
            TDF_Label deletedFacesLabel      = myBoxCutLabel.FindChild(3);
            TDF_Label intersectingFacesLabel = myBoxCutLabel.FindChild(4);
            TDF_Label newFacesLabel          = myBoxCutLabel.FindChild(5);

            // Finally time to store the result of the Cut operation on Child 1 of myBoxCutLabel -> we called this label
            // resultCutLabel earlier
            TNaming_Builder resultBoxBuilder(resultCutLabel);
            const TopoDS_Shape& preCutBox = mkCut.Shape1(); // first shape involved in boolean operation, i.e. myBox
            // The two arguments here are OldShape and NewShape, respectively. This stores in resultCutLabel a Modified
            // attribute wich describes NewShape as a Modification of OldShape
            resultBoxBuilder.Modify(preCutBox, resultBox);

            // Going one level deeper, we need to store in the Data Framework the modified faces.
            TopTools_MapOfShape uniqueShapes;
            TNaming_Builder modifiedFacesBuilder(modifiedFacesLabel);

            // Create a TopExp_Explorer to traverse the FACE's of the original Box
            TopExp_Explorer FaceExplorer(preCutBox, TopAbs_FACE);

            // Alright, let's work through the logic on this nested For-loop structure. First, it loops through each
            // TopoDS_FACE in the preCutBox, i.e. the original Box before it was cut. For each Face in the preCutBox,
            // use the BRepAlgo_Cut instance we have from earlier, mkCut, to check whether or not that Face was Modified
            // during the Cut operation. In fact, mkCut.Modified returns a _list_ of Modified Shapes. Why? Hm..I think
            // it's b/c the Face that we're checking _could_ have gotten split into multiple Faces during the Cut
            // operation, and therefore the Modified check needs to give allowance for that. Not really sure though...
            // But, you know, assuming that's true, the next step is to iterate over overy one of these Modified Face's.
            // At each step of this second iteration, we make sure this possiblyModifiedFace is not the same as
            // currentFace before adding it to the Data Framework. 
            //
            // Why do we have to preform this check? And why doesn't currentFace itself get added to the Data Framework?
            // I don't know! Lol, in fact, I'm a bit confused myself. It seems to me that even if the Face returned from
            // mkCut.Modified _is_ the same as currentFace, that we'd still want to add it to the Data Framework, since,
            // well, it was Modified. For now, I'll blindly follow the OCAF example, and hopefully I'll become
            // enlightened as to what is going on here...
            for (; FaceExplorer.More(); FaceExplorer.Next()){
                const TopoDS_Shape& currentFace = FaceExplorer.Current();
                Standard_Boolean res = uniqueShapes.Add(currentFace);
                if (!res)
                    // Face wasn't added, I think b/c it was already in the Map. Therefore, skip to the next Face. I
                    // think in FreeCAD we use TopExp_MapShapes instead of this, not sure why the OCAF sample does this.
                    // For now we'll do it their way...
                    continue;
                const TopTools_ListOfShape& modifiedFaces = mkCut.Modified(currentFace);
                TopTools_ListIteratorOfListOfShape modifiedFacesIterator(modifiedFaces);
                for (; modifiedFacesIterator.More(); modifiedFacesIterator.Next()){
                    const TopoDS_Shape& possiblyModifiedFace = modifiedFacesIterator.Value();
                    if (!currentFace.IsSame(possiblyModifiedFace))
                        modifiedFacesBuilder.Modify(currentFace, possiblyModifiedFace);
                }
            }

            // Do the same as above for Deleted faces
            uniqueShapes.Clear();
            TNaming_Builder deletedFacesBuilder(deletedFacesLabel);
            FaceExplorer.Init(preCutBox, TopAbs_FACE);
            // So, why is this loop simpler than the Modified loop? My best guess is that, unlike with Modified, a
            // Deleted Face does not ever result in multiple Faces being produces - I THINK! Instead, a Face is either
            // Deleted or it's not. *shrug*
            for(; FaceExplorer.More(); FaceExplorer.Next()){
                const TopoDS_Shape& currentFace = FaceExplorer.Current();
                Standard_Boolean res = uniqueShapes.Add(currentFace);
                if (!res)
                    continue;
                if (mkCut.IsDeleted(currentFace))
                    deletedFacesBuilder.Delete(currentFace);
            }

            // Add to the Data Framework 'section edges'. I'm not gonna lie, don't really understand this whole
            // 'Intersecting Faces' thing, but it has something to do with the Boolean operation
            TNaming_Builder intersectingFacesBuilder(intersectingFacesLabel);
            Handle(TopOpeBRepBuild_HBuilder) build = mkCut.Builder();
            TopTools_ListIteratorOfListOfShape intersectingSections = build->Section();
            for(; intersectingSections.More(); intersectingSections.Next()){
                // So, Select I guess adds a Shape to the label - a shape that wasn't modified. Pretty confused about
                // this, why does it need two arguements?
                intersectingFacesBuilder.Select(intersectingSections.Value(), intersectingSections.Value());
            }

            // Finally, add to the Data Framework faces that were added
            const TopoDS_Shape& boxThatWasCutOut = mkCut.Shape2();
            TNaming_Builder newFacesBuilder(newFacesLabel);
            FaceExplorer.Init(boxThatWasCutOut, TopAbs_FACE);
            for (; FaceExplorer.More(); FaceExplorer.Next()){
                const TopoDS_Shape& currentFace = FaceExplorer.Current();
                const TopTools_ListOfShape& modifiedFaces = mkCut.Modified(currentFace);
                if (!modifiedFaces.IsEmpty()){
                    TopTools_ListIteratorOfListOfShape modifiedFacesIterator(modifiedFaces);
                    for (; modifiedFacesIterator.More(); modifiedFacesIterator.Next()){
                        const TopoDS_Shape& newShape = modifiedFacesIterator.Value();
                        // Uhm, the documentation has a lot to say about this NamedShape method. :-/
                        Handle(TNaming_NamedShape) aNamedShape = TNaming_Tool::NamedShape(newShape, newFacesLabel);
                        if (aNamedShape.IsNull() || aNamedShape->Evolution() != TNaming_MODIFY){
                            newFacesBuilder.Generated(currentFace, newShape);
                        }
                    }
                }
            }
        }
    }

    // PHEW! Maybe one day I'll understand all of that... For now, let's just accept that we're done with the CUT
    // operation: we've cut stuff, and we've stored all the necessary data in the Data Framework. So, this means that,
    // although the Indexes between the Faces of myBox and resultBox may be different, I SHOULD be able to use the
    // Selector thing from earlier to get a constant reference to a given face...
    printShapeInfos(myBox, cutBox, resultBox);

    // NOTE: if you pipe this output into a file, say "./occTest > tdfShapes.txt" you can then feed that txt file to the
    // parser2.py that I posted on the Forum "python parser2.py tdfShapes.txt" to get a side-by-side comparison

    // Success! Well, sort of. The Face indexes between myBox and resultBox are indeed different. So, now let's take
    // this Data Framework thing for a test drive...
    
    // scratch that, move on to selector
    //// First, let's ignore that Selector thing and see ho good this FindAttribute thing works
    //Handle(TNaming_NamedShape) namedShapeObject; // store the retrieved object here
    //myBoxLabel.FindAttribute(TNaming_NamedShape::GetID(), namedShapeObject);
    //TopoDS_Shape myRecoveredBox = namedShapeObject->Get();

    //TDF_Label myTestingNodeLabel = aLabel.FindChild(4, createIfNotExists);
    //TDF_Label myTopFaceLabel     = myTestingNodeLabel.FindChild(1, createIfNotExists);

    //TNaming_Selector myTopFaceSelector(resultCutLabel); // creates a selector at this Label

    Handle(TNaming_NamedShape) topFaceNamedShape;
    // myBoxTopLabel was defined waaaaay at the beginning of all this, when we first created myBox, before we Cut
    // anything out of it. This FindAttribute stores the Shape associated with this label in topFaceNamedShape
    myBoxTopLabel.FindAttribute(TNaming_NamedShape::GetID(), topFaceNamedShape);
    const TopoDS_Shape& theTopFace = TNaming_Tool::CurrentShape(topFaceNamedShape);
    std::cout << "The shape info for the recovered top face" << std::endl;
    printShapeInfo(theTopFace);
}

int main(){
    //runCase1();
    //runCase2();
    runCase3();
    return 0;
}
