#include <iostream>

#include <TCollection_AsciiString.hxx>

#include <TDataStd_AsciiString.hxx>

#include <TDF_Data.hxx>
#include <TDF_Label.hxx>
#include <TDF_TagSource.hxx>
#include <TDF_LabelMap.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Tool.hxx>
#include <TDF_MapIteratorOfLabelMap.hxx>
#include <TDF_IDFilter.hxx>
#include <TDF_AttributeIndexedMap.hxx>

#include <TNaming_NamedShape.hxx>
#include <TNaming_UsedShapes.hxx>
#include <TNaming_Selector.hxx>
#include <TNaming_Tool.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming.hxx>

#include <Geom_Plane.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom_TrimmedCurve.hxx>

#include <GC_MakeSegment.hxx>


#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <TopLoc_Location.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_MapIteratorOfMapOfShape.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>

#include <TopOpeBRepBuild_HBuilder.hxx>

#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>

#include <BRepAlgo.hxx>
#include <BRepAlgo_Cut.hxx>
#include <BRepAlgo_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>

#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Vec.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>
#include <gp_Trsf.hxx>

void printShapeInfos(TopoDS_Shape shape1, TopoDS_Shape shape2, TopoDS_Shape shape3);
void writeShapeInfos(TopoDS_Shape shape1, TopoDS_Shape shape2, TopoDS_Shape shape3);
void printShapeInfo(TopoDS_Shape shape, TopAbs_ShapeEnum which=TopAbs_FACE);
void writeShapeInfo(TopoDS_Shape shape, std::ofstream&);
void printDumpFile(TopoDS_Shape shape);
void writeDumpFiles(TopoDS_Shape orig, TopoDS_Shape cut, TopoDS_Shape newBox);
void AddTextToLabel(TDF_Label& Label, char* str);
void AddTextToLabel(TDF_Label& Label, std::string Text);
TopoDS_Shape MakeTrackedBox(const Standard_Real dx, const Standard_Real dy,
                            const Standard_Real dz, const TDF_Label LabelRoot);
TopoDS_Shape MakeTrackedCyl(const Standard_Real rad, const Standard_Real height,
                            const TDF_Label LabelRoot);
TopoDS_Shape MakeTrackedTransform(TopoDS_Shape Shape, gp_Trsf Transformation, TDF_Label& LabelRoot);
void MakeTrackedCut(TopoDS_Shape BaseShape, TopoDS_Shape CutShape, TDF_Label& LabelRoot);
TopoDS_Shape MakeTrackedFuse(TopoDS_Shape BaseShape, TopoDS_Shape AddShape, TDF_Label& LabelRoot);
void MakeTrackedSelection(TopoDS_Shape BaseShape, TopoDS_Shape SelectedShape, TDF_Label& LabelRoot);
void MakeTrackedSelection(TopoDS_Shape BaseShape, TopTools_IndexedMapOfShape Selections, TDF_Label& LabelRoot);
void MakeTrackedFillets(TopoDS_Shape BaseShape, TopTools_IndexedMapOfShape Edges, TDF_Label& FilletLabelRoot);

#include "ArchivedRunCases.cpp"

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

void printShapeInfo(TopoDS_Shape shape, TopAbs_ShapeEnum which){
    TopTools_IndexedMapOfShape mapOfShapes;
    TopExp::MapShapes(shape, which, mapOfShapes);
    if (which == TopAbs_FACE){
        for (int i = 1; i <= mapOfShapes.Extent(); i++){
            TopoDS_Face aFace = TopoDS::Face(mapOfShapes.FindKey(i));
            Handle(Geom_Surface) aSurface = BRep_Tool::Surface(aFace);
            if(aSurface->IsKind(STANDARD_TYPE(Geom_Plane))){
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
                std::cout << "For i= " << i << ", ";
                std::cout << "type = Geom_Plane, ";
                std::cout << "loc = (" << locx << ", " << locy << ", " << locz << ") , ";
                std::cout << "dir = (" << dirx << ", " << diry << ", " << dirz << ") " << std::endl;
            }
            else if(aSurface->IsKind(STANDARD_TYPE(Geom_CylindricalSurface))){
                std::cout << "It is a cylindrical surface" << std::endl;
                Handle(Geom_CylindricalSurface) aCylSurface = Handle(Geom_CylindricalSurface)::DownCast(aSurface);
                gp_Pnt aPoint = aCylSurface->Location();
                gp_Ax1 anAxis = aCylSurface->Axis();
                gp_Dir aDir   = anAxis.Direction();
                Standard_Real locx, locy, locz, dirx, diry, dirz;
                locx = aPoint.X();
                locy = aPoint.Y();
                locz = aPoint.Z();
                dirx = aDir.X();
                diry = aDir.Y();
                dirz = aDir.Z();
                std::cout << "For i= " << i << " ";
                std::cout << "type = Geom_CylindricalSurface, ";
                std::cout << "loc = (" << locx << ", " << locy << ", " << locz << ") , ";
                std::cout << "dir = (" << dirx << ", " << diry << ", " << dirz << ") " << std::endl;
            }
            else{
                std::cout << "Surface not a Plane at i = " << i;
                std::cout << " The DynamicType is = " << aSurface->DynamicType() << std::endl;
            }
        }
    }
    else if(which == TopAbs_EDGE){
        for (int i = 1; i <= mapOfShapes.Extent(); i++){
            Standard_Real lineStart, lineEnd;
            TopoDS_Edge anEdge = TopoDS::Edge(mapOfShapes.FindKey(i));
            Handle(Geom_Curve) aCurve = BRep_Tool::Curve(anEdge, lineStart, lineEnd);
            if (aCurve->DynamicType() == STANDARD_TYPE(Geom_Line)){
                Handle(Geom_Line) aLine = Handle(Geom_Line)::DownCast(aCurve);
                gp_Pnt point1, point2;
                aLine->D0(lineStart, point1);
                aLine->D0(lineEnd, point2);
                gp_Ax1 anAxis = aLine->Position();
                gp_Dir aDir   = anAxis.Direction();
                std::cout << "For i= " << i << " ";
                std::cout << "type = Geom_Edge, ";
                std::cout << "point1 = (" << point1.X() << ", " << point1.Y() << ", " << point1.Z() << ") , ";
                std::cout << "point2 = (" << point2.X() << ", " << point2.Y() << ", " << point2.Z() << ") , ";
                std::cout << "dir = (" << aDir.X() << ", " << aDir.Y() << ", " << aDir.Z() << ") " << std::endl;
            }
            else if (aCurve->DynamicType() == STANDARD_TYPE(Geom_Circle)){
                Handle(Geom_Circle) aCircle = Handle(Geom_Circle)::DownCast(aCurve);
                Standard_Real radius, locx, locy, locz;
                radius = aCircle->Radius();
                gp_Ax1 anAxis = aCircle->Axis();
                gp_Dir aDir   = anAxis.Direction();
                gp_Pnt aPoint = aCircle->Location();
                locx = aPoint.X();
                locy = aPoint.Y();
                locz = aPoint.Z();
                std::cout << "For i= " << i << " ";
                std::cout << "type = Geom_Circle, ";
                std::cout << "radius = " << radius << " ";
                std::cout << "loc = (" << locx << ", " << locy << ", " << locz << ") , ";
                std::cout << "dir = (" << aDir.X() << ", " << aDir.Y() << ", " << aDir.Z() << ") " << std::endl;
            }
            else{
                std::cout << "Curve not a Line or Circle at i = " << i;
                std::cout << " The DynamicType is = " << aCurve->DynamicType() << std::endl;
            }
        }
    }
    else{
        std::cout << "Don't recognize that TopAbs, sorry" << std::endl;
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

void AddTextToLabel(TDF_Label& Label, char const *str){
    Handle(TDataStd_AsciiString) nameAttribute;
    TCollection_AsciiString myName;
    myName = str;
    nameAttribute = new TDataStd_AsciiString();
    nameAttribute->Set(myName);
    Label.AddAttribute(nameAttribute);
}
void AddTextToLabel(TDF_Label& Label, std::string Text){
    AddTextToLabel(Label, Text.c_str());
}

TopoDS_Shape MakeTrackedBox(const Standard_Real dx, const Standard_Real dy,
                            const Standard_Real dz, const TDF_Label LabelRoot){
    //TDF_Label LabelRoot = *LabelRootPtr;
    BRepPrimAPI_MakeBox MakeBox( dx, dy, dz); 
    TopoDS_Shape GendBox = MakeBox.Shape();

    // create the labels we'll need
    TDF_Label Top1      = TDF_TagSource::NewChild(LabelRoot);
    TDF_Label Bottom1   = TDF_TagSource::NewChild(LabelRoot);
    TDF_Label Right1    = TDF_TagSource::NewChild(LabelRoot);
    TDF_Label Left1     = TDF_TagSource::NewChild(LabelRoot);
    TDF_Label Front1    = TDF_TagSource::NewChild(LabelRoot);
    TDF_Label Back1     = TDF_TagSource::NewChild(LabelRoot);

    // add the generated box to the LabelRoot
    TNaming_Builder GeneratedBoxBuilder(LabelRoot);
    GeneratedBoxBuilder.Generated(GendBox);
    //std::ostringstream text;
    //text << "Box, dx=" << dx << ", dy=" << dy << " dz=" << dz << std::endl;
    //AddTextToLabel(LabelRoot, "Box created");

    TNaming_Builder Top1FaceIns (Top1);
    TopoDS_Face Top1Face = MakeBox.TopFace ();
    Top1FaceIns.Generated (Top1Face);  

    TopoDS_Face Bottom1Face = MakeBox.BottomFace ();
    TNaming_Builder Bottom1FaceIns (Bottom1); 
    Bottom1FaceIns.Generated (Bottom1Face);

    TopoDS_Face Right1Face = MakeBox.RightFace ();
    TNaming_Builder Right1FaceIns (Right1); 
    Right1FaceIns.Generated (Right1Face); 

    TopoDS_Face Left1Face = MakeBox.LeftFace ();
    TNaming_Builder Left1FaceIns (Left1); 
    Left1FaceIns.Generated (Left1Face); 

    TopoDS_Face Front1Face = MakeBox.FrontFace ();
    TNaming_Builder Front1FaceIns (Front1);
    Front1FaceIns.Generated (Front1Face); 

    TopoDS_Face Back1Face = MakeBox.BackFace ();
    TNaming_Builder Back1FaceIns (Back1); 
    Back1FaceIns.Generated (Back1Face); 

    return GendBox;
}

TopoDS_Shape MakeTrackedCyl(const Standard_Real rad, const Standard_Real height,
                            const TDF_Label LabelRoot){
    BRepPrimAPI_MakeCylinder MakeCylinder(rad, height);
    TopoDS_Shape GendCyl = MakeCylinder.Shape();

    //create the labels we'll need
    TDF_Label SeamEdge     = TDF_TagSource::NewChild(LabelRoot);
    TDF_Label Top          = TDF_TagSource::NewChild(LabelRoot);
    TDF_Label Bottom       = TDF_TagSource::NewChild(LabelRoot);
    TDF_Label Side         = TDF_TagSource::NewChild(LabelRoot);

    // add the generated cylinder to the LabelRoot
    TNaming_Builder GeneratedCylBuilder(LabelRoot);
    GeneratedCylBuilder.Generated(GendCyl);

    TopoDS_Shape BotFace, TopFace;
    TopTools_IndexedMapOfShape mapOfFaces;
    TopExp::MapShapes(GendCyl, TopAbs_FACE, mapOfFaces);
    int i=1;
    int maxZ = -1;
    for (; i<=mapOfFaces.Extent(); i++){
        TopoDS_Face aFace = TopoDS::Face(mapOfFaces.FindKey(i));
        Handle(Geom_Surface) aSurface = BRep_Tool::Surface(aFace);

        if(aSurface->IsKind(STANDARD_TYPE(Geom_Plane))){
            Handle(Geom_Plane) aPlane = Handle(Geom_Plane)::DownCast(aSurface);
            gp_Pnt location = aPlane->Location();
            if (location.Z() > maxZ){
                if (!TopFace.IsNull()){
                    BotFace = TopFace;
                }
                TopFace = aFace;
                maxZ = location.Z();
            }
            else{
                if(!BotFace.IsNull()){
                    TopFace = BotFace;
                }
                BotFace = aFace;
            }
        }
        else if(aSurface->IsKind(STANDARD_TYPE(Geom_CylindricalSurface))){
            TNaming_Builder SideBuilder(Side);
            SideBuilder.Generated(aFace);
            Handle(Geom_CylindricalSurface) aCylSurface = Handle(Geom_CylindricalSurface)::DownCast(aSurface);
            gp_Pnt aPoint = aCylSurface->Location();
            gp_Ax1 anAxis = aCylSurface->Axis();
            gp_Dir aDir   = anAxis.Direction();
            Standard_Real locx, locy, locz, dirx, diry, dirz;
            locx = aPoint.X();
            locy = aPoint.Y();
            locz = aPoint.Z();
        }
    }

    TNaming_Builder Top1FaceIns (Top);
    Top1FaceIns.Generated (TopFace);  

    TNaming_Builder Bottom1FaceIns (Bottom);
    Bottom1FaceIns.Generated (BotFace);

    TopTools_IndexedMapOfShape mapOfEdges;
    TopExp::MapShapes(GendCyl, TopAbs_EDGE, mapOfEdges);
    i=1;
    for (; i<=mapOfFaces.Extent(); i++){
        TopoDS_Edge anEdge = TopoDS::Edge(mapOfEdges.FindKey(i));
        Standard_Real lineStart, lineEnd;
        Handle(Geom_Curve) aCurve = BRep_Tool::Curve(anEdge, lineStart, lineEnd);

        if (aCurve->DynamicType() == STANDARD_TYPE(Geom_Line)){
            std::cout << "Added seam edge" << std::endl;
            TNaming_Builder SeamBuilder(SeamEdge);
            SeamBuilder.Generated(anEdge);
        }
    }

    return MakeCylinder.Shape();
}

TopoDS_Shape MakeTrackedTransform(TopoDS_Shape Shape, gp_Trsf Transformation, TDF_Label& LabelRoot){
    TopLoc_Location location(Transformation);
    TDF_LabelMap scope;
    TDF_ChildIterator itchild;
    for (itchild.Initialize(LabelRoot, Standard_True); itchild.More();itchild.Next()) {
        if (itchild.Value().IsAttribute(TNaming_NamedShape::GetID()))
            scope.Add(itchild.Value());
    }
    if (LabelRoot.IsAttribute(TNaming_NamedShape::GetID()))
        scope.Add(LabelRoot);
    TDF_MapIteratorOfLabelMap it(scope);
    for (;it.More();it.Next()) 
        TNaming::Displace(it.Key(), location, Standard_True);//with oldshapes

    // Get back the translated box to return
    Handle(TNaming_NamedShape) MovedBoxNS;
    LabelRoot.FindAttribute(TNaming_NamedShape::GetID(), MovedBoxNS);
    return MovedBoxNS->Get();
}

void MakeTrackedCut(TopoDS_Shape BaseShape, TopoDS_Shape CutShape, TDF_Label& LabelRoot){
    TDF_Label Tool          = TDF_TagSource::NewChild(LabelRoot);
    TDF_Label Modified      = TDF_TagSource::NewChild(LabelRoot); 
    TDF_Label Deleted       = TDF_TagSource::NewChild(LabelRoot); 
    TDF_Label Intersections = TDF_TagSource::NewChild(LabelRoot); 
    TDF_Label NewFaces      = TDF_TagSource::NewChild(LabelRoot); 

     //Select the 'cut shape', for some reason
    TNaming_Selector ToolSelector2(Tool);
    ToolSelector2.Select(CutShape, CutShape);

    BRepAlgo_Cut MyCutter(BaseShape, CutShape);
    TopoDS_Shape ResultShape = MyCutter.Shape();

    // push CUT results in DF as modification of Box1
    TNaming_Builder resultBuilder (LabelRoot);
    resultBuilder.Modify (BaseShape, ResultShape);

    //push in the DF modified faces

    TNaming_Builder ModBuilder2(Modified);
    TopTools_IndexedMapOfShape mapOfShapes;
    TopExp::MapShapes(BaseShape, TopAbs_FACE, mapOfShapes);
    int i=1;
    for (; i<=mapOfShapes.Extent(); i++){
        const TopoDS_Shape& Root = mapOfShapes.FindKey(i);
        const TopTools_ListOfShape& Shapes = MyCutter.Modified (Root);
        TopTools_ListIteratorOfListOfShape ShapesIterator (Shapes);
        for (;ShapesIterator.More (); ShapesIterator.Next ()) {
            const TopoDS_Shape& newShape = ShapesIterator.Value ();
            // TNaming_Evolution == MODIFY
            if (!Root.IsSame (newShape))
                ModBuilder2.Modify (Root,newShape );
        }
    }

    //push in the DF deleted faces
    TNaming_Builder DelBuilder2(Deleted);
    mapOfShapes.Clear();
    TopExp::MapShapes(BaseShape, TopAbs_FACE, mapOfShapes);
    i=1;
    for (; i<=mapOfShapes.Extent(); i++){
        const TopoDS_Shape& Root = mapOfShapes.FindKey(i);
        if (MyCutter.IsDeleted (Root))
            DelBuilder2.Delete (Root);
    }

    // push in the DF section edges
    TNaming_Builder IntersBuilder2(Intersections);
    Handle(TopOpeBRepBuild_HBuilder) build = MyCutter.Builder();  
    TopTools_ListIteratorOfListOfShape its = build->Section();
    for (; its.More(); its.Next()) {
        IntersBuilder2.Select(its.Value(),its.Value());
    }

    // push in the DF new faces added to the object:
    TNaming_Builder newBuilder2 (NewFaces);
    mapOfShapes.Clear();
    TopExp::MapShapes(MyCutter.Shape2(), TopAbs_FACE, mapOfShapes);
    i=1;
    for (; i<=mapOfShapes.Extent(); i++){
        const TopoDS_Shape& F = mapOfShapes.FindKey(i);
        const TopTools_ListOfShape& modified = MyCutter.Modified(F);
        if (!modified.IsEmpty()) {
            TopTools_ListIteratorOfListOfShape itr(modified);
            for (; itr.More (); itr.Next ()) {
                const TopoDS_Shape& newShape = itr.Value();
                Handle(TNaming_NamedShape) NS = TNaming_Tool::NamedShape(newShape, NewFaces);
                if (NS.IsNull() || NS->Evolution() != TNaming_MODIFY) {
                    // TNaming_Evolution == GENERATED
                    newBuilder2.Generated(F, newShape); 	
                } // if (NS.IsNul())...
            } // for (; itr.More()...
        } // if (!modified.IsEmpty()...
    } //for (; ShapeExplorer.More()...
}

TopoDS_Shape MakeTrackedFuse(TopoDS_Shape BaseShape, TopoDS_Shape AddShape, TDF_Label& LabelRoot){
    TDF_Label Tool              = TDF_TagSource::NewChild(LabelRoot);
    TDF_Label ModifiedBase      = TDF_TagSource::NewChild(LabelRoot); 
    TDF_Label ModifiedAdd       = TDF_TagSource::NewChild(LabelRoot); 

     //Select the 'add shape', for some reason
    TNaming_Selector ToolSelector2(Tool);
    ToolSelector2.Select(AddShape, AddShape);

    BRepAlgo_Fuse MyFuser(BaseShape, AddShape);
    TopoDS_Shape ResultShape = MyFuser.Shape();

    // push FUSED results in DF as modification of Box1
    TNaming_Builder resultBuilder (LabelRoot);
    resultBuilder.Modify (BaseShape, ResultShape);

    //push in the DF modified faces on Base Shape

    TNaming_Builder ModBaseBuilder(ModifiedBase);
    TopTools_IndexedMapOfShape mapOfShapes;
    TopExp::MapShapes(BaseShape, TopAbs_FACE, mapOfShapes);
    int i=1;
    for (; i<=mapOfShapes.Extent(); i++){
        const TopoDS_Shape& Root = mapOfShapes.FindKey(i);
        const TopTools_ListOfShape& Shapes = MyFuser.Modified (Root);
        TopTools_ListIteratorOfListOfShape ShapesIterator (Shapes);
        for (;ShapesIterator.More (); ShapesIterator.Next ()) {
            const TopoDS_Shape& newShape = ShapesIterator.Value ();
            // TNaming_Evolution == MODIFY
            if (!Root.IsSame (newShape))
                ModBaseBuilder.Modify (Root,newShape );
        }
    }

    //push in the DF modified faces on Add Shape

    TNaming_Builder ModAddBuilder(ModifiedAdd);
    TopExp::MapShapes(AddShape, TopAbs_FACE, mapOfShapes);
    i=1;
    for (; i<=mapOfShapes.Extent(); i++){
        const TopoDS_Shape& Root = mapOfShapes.FindKey(i);
        const TopTools_ListOfShape& Shapes = MyFuser.Modified (Root);
        TopTools_ListIteratorOfListOfShape ShapesIterator (Shapes);
        for (;ShapesIterator.More (); ShapesIterator.Next ()) {
            const TopoDS_Shape& newShape = ShapesIterator.Value ();
            // TNaming_Evolution == MODIFY
            if (!Root.IsSame (newShape))
                ModAddBuilder.Modify (Root,newShape );
        }
    }

    return ResultShape;
}


void MakeTrackedSelection(TopoDS_Shape BaseShape, TopoDS_Shape SelectedShape, TDF_Label& LabelRoot){
    const TDF_Label& SelEdge  = TDF_TagSource::NewChild(LabelRoot);
    TNaming_Selector Selector(SelEdge);
    Selector.Select(SelectedShape, BaseShape);
}

void MakeTrackedSelection(TopoDS_Shape BaseShape, TopTools_IndexedMapOfShape Selections, TDF_Label& LabelRoot){
    Standard_Integer i=1;
    for(; i<= Selections.Extent(); i++){
        const TopoDS_Shape& Shape = Selections(i);
        const TDF_Label& SelEdge  = TDF_TagSource::NewChild(LabelRoot);
        TNaming_Selector Selector(SelEdge);

        Selector.Select(Shape, BaseShape);
    }
}

void MakeTrackedFillets(TopoDS_Shape BaseShape, TopTools_IndexedMapOfShape Edges, TDF_Label& FilletLabelRoot){
    //// First, SELECT each edge that we'll fillet, for future reference
    //MakeTrackedSelection(BaseShape, Edges, FilletLabelRoot);

    // Now, Perform the Fillet operation
    BRepFilletAPI_MakeFillet MakeFillet(BaseShape);// fillet's algo

    Standard_Integer i=1;
    for(; i<= Edges.Extent(); i++){
        const TopoDS_Edge& E = TopoDS::Edge(Edges(i));
        MakeFillet.Add(5., 5., E);
    }
    MakeFillet.Build();

    if(!MakeFillet.IsDone()){
        std::cout << "fillet failed, bailing out" << std::endl;
        return; //Algorithm failed
    }

    TopoDS_Shape ResultShape = MakeFillet.Shape();

    // Finally, store all the necessary information

    TDF_Label DeletedFaces      = TDF_TagSource::NewChild(FilletLabelRoot);
    TDF_Label ModifiedFaces     = TDF_TagSource::NewChild(FilletLabelRoot);
    TDF_Label FacesFromEdges    = TDF_TagSource::NewChild(FilletLabelRoot);
    TDF_Label FacesFromVertices = TDF_TagSource::NewChild(FilletLabelRoot);

    // TNaming_Evolution == MODIFY
    TNaming_Builder ResultBuilder(FilletLabelRoot);
    ResultBuilder.Modify(BaseShape, ResultShape);
    AddTextToLabel(FilletLabelRoot, "FilletLabelRoot");

    //New faces generated from edges
    TNaming_Builder FaceFromEdgeBuilder(FacesFromEdges);  
    TopTools_IndexedMapOfShape mapOfEdges;
    TopExp::MapShapes(BaseShape, TopAbs_EDGE, mapOfEdges);
    i=1;
    for (; i <= mapOfEdges.Extent(); i++){
        const TopoDS_Shape& CurrentEdge = mapOfEdges.FindKey(i);
        // I gues the MakeFillet algo takes an edge and sweeps it or something
        const TopTools_ListOfShape& Shapes = MakeFillet.Generated (CurrentEdge);
        TopTools_ListIteratorOfListOfShape ShapesIterator (Shapes);
        for (;ShapesIterator.More (); ShapesIterator.Next ()) {
            const TopoDS_Shape& newShape = ShapesIterator.Value ();
            // I guess the edge also get's re-generated?
            if (!CurrentEdge.IsSame (newShape)){
                FaceFromEdgeBuilder.Generated (CurrentEdge, newShape );
                std::cout << "Found a generated Shape, i = " << i << std::endl;
                printShapeInfo(CurrentEdge);
            }
        }
    }

    //Faces of the initial shape modified by MakeFillet
    TNaming_Builder ModFacesBuilder(ModifiedFaces);
    TopTools_IndexedMapOfShape mapOfFaces;
    TopExp::MapShapes(BaseShape, TopAbs_FACE, mapOfFaces);
    i=1;
    for (; i <= mapOfFaces.Extent(); i++){
        const TopoDS_Shape& CurrentFace = mapOfFaces.FindKey(i);
        const TopTools_ListOfShape& Shapes = MakeFillet.Modified (CurrentFace);
        TopTools_ListIteratorOfListOfShape ShapesIterator (Shapes);
        for (;ShapesIterator.More (); ShapesIterator.Next ()) {
            const TopoDS_Shape& newShape = ShapesIterator.Value ();
            // Not sure how they could be the same...
            if (!CurrentFace.IsSame (newShape))
                ModFacesBuilder.Modify (CurrentFace,newShape );
                std::cout << "Found Modified Shape, i= " << i << std::endl;
                printShapeInfo(CurrentFace);
        }
    }

    //Deleted faces of the initial shape
    TNaming_Builder DelFacesBuilder(DeletedFaces);
    TopExp::MapShapes(BaseShape, TopAbs_FACE, mapOfFaces);
    i=1;
    for (; i<=mapOfFaces.Extent(); i++){
        const TopoDS_Shape& CurrentFace = mapOfFaces.FindKey(i);
        // TNaming_Evolution == DELETE
        if (MakeFillet.IsDeleted (CurrentFace)){
            DelFacesBuilder.Delete(CurrentFace);
            std::cout << "Found Deleted Shape, i= " << i << std::endl;
            printShapeInfo(CurrentFace);
        }
    }

    //New faces generated from vertices
    TNaming_Builder FaceFromVertexBuilder(FacesFromVertices);
    TopTools_IndexedMapOfShape mapOfVertexes;
    TopExp::MapShapes(BaseShape, TopAbs_VERTEX, mapOfVertexes);
    i=1;
    for (; i<=mapOfVertexes.Extent(); i++){
        const TopoDS_Shape& CurrentVertex = mapOfVertexes.FindKey(i);
        const TopTools_ListOfShape& Shapes = MakeFillet.Generated (CurrentVertex);
        TopTools_ListIteratorOfListOfShape ShapesIterator (Shapes);
        for (;ShapesIterator.More (); ShapesIterator.Next ()) {
            const TopoDS_Shape& newShape = ShapesIterator.Value ();
            // TNaming_Evolution == GENERATED
            if (!CurrentVertex.IsSame (newShape)){
                FaceFromVertexBuilder.Generated (CurrentVertex, newShape );
                std::cout << "Found Shape generated by Vertex, i = " << i << std::endl;
            }
        }
    }
}

void runCase5(){
    std::cout << "Running case 5" << std::endl;
    
    // Create the Data Framework and Root node
    Handle(TDF_Data) DF          = new TDF_Data();
    const TDF_Label MyRoot       = DF->Root();
    TDF_Label Box1Label          = TDF_TagSource::NewChild(MyRoot); // 1
    TDF_Label Box2Label          = TDF_TagSource::NewChild(MyRoot); // 2
    TDF_Label SelEdgesLabel      = TDF_TagSource::NewChild(MyRoot); // 3
    TDF_Label FilletedBox1Label  = TDF_TagSource::NewChild(MyRoot); // 4
    TDF_Label Box1CutLabel       = TDF_TagSource::NewChild(MyRoot); // 5

    // Create both boxes
    TopoDS_Shape Box1 = MakeTrackedBox(100., 100., 100., Box1Label);
    TopoDS_Shape Box2 = MakeTrackedBox(150., 150., 150., Box2Label);

    // Move the second box
    gp_Vec vec1(gp_Pnt(0.,0.,0.),gp_Pnt(50.,50.,20.));
    gp_Trsf Transformation;
    Transformation.SetTranslation(vec1);
    Box2 = MakeTrackedTransform(Box2, Transformation, Box2Label);

    // Select the edges we're going to fillet

    // Is there a better way to keep track of which child is which Face?
    Handle(TNaming_NamedShape) Box1TopFaceNS;
    Box1Label.FindChild(1).FindAttribute(TNaming_NamedShape::GetID(), Box1TopFaceNS);
    const TopoDS_Shape& top1face  = TNaming_Tool::GetShape(Box1TopFaceNS);
    TopTools_IndexedMapOfShape mapOfEdges;
    TopExp::MapShapes(top1face, TopAbs_EDGE, mapOfEdges);
    MakeTrackedSelection(Box1, mapOfEdges, SelEdgesLabel);

    // Make the fillet operation
    MakeTrackedFillets(Box1, mapOfEdges, FilletedBox1Label);

    // make the cut operation.
    MakeTrackedCut(Box1, Box2, Box1CutLabel);

    TDF_Tool::DeepDump(std::cout, DF);
}

void runFilletBug(){
    std::cout << "Running case FilletBug" << std::endl;
    
    // Create the Data Framework and Root node
    Handle(TDF_Data) DF                = new TDF_Data();
    const TDF_Label MyRoot             = DF->Root();
    TDF_Label Box1Label                = TDF_TagSource::NewChild(MyRoot); // 1
    TDF_Label CylinderLabel            = TDF_TagSource::NewChild(MyRoot); // 2
    TDF_Label FusedSolidLabel          = TDF_TagSource::NewChild(MyRoot); // 3
    TDF_Label SelEdgesLabel            = TDF_TagSource::NewChild(MyRoot); // 4
    TDF_Label FilletedBox1Label        = TDF_TagSource::NewChild(MyRoot); // 5
    TDF_Label Box1CutLabel             = TDF_TagSource::NewChild(MyRoot); // 6

    // Create Box and Cylinder
    TopoDS_Shape Box1 = MakeTrackedBox(100., 100., 100., Box1Label);
    TopoDS_Shape Cyl1 = MakeTrackedCyl(25., 100., CylinderLabel);

    // Fuse two shapes
    TopoDS_Shape FusedShape = MakeTrackedFuse(Box1, Cyl1, FusedSolidLabel);
    TDF_Tool::DeepDump(std::cout, DF);
}

int main(){
    //runCase1();
    //runCase2();
    //runCase3();
    //runCase4();
    //runCase5();
    runFilletBug();
    return 0;
}
