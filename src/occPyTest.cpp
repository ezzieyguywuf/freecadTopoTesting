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
#include <Geom_Line.hxx>
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
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>

#include <BRepAlgo.hxx>
#include <BRepAlgo_Cut.hxx>
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
void MakeTrackedCut(TopoDS_Shape BaseShape, TopoDS_Shape CutShape, TDF_Label& LabelRoot);

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
                std::cout << "point1 = (" << point1.X() << ", " << point1.Y() << ", " << point1.Z() << ") , ";
                std::cout << "point2 = (" << point2.X() << ", " << point2.Y() << ", " << point2.Z() << ") , ";
                std::cout << "dir = (" << aDir.X() << ", " << aDir.Y() << ", " << aDir.Z() << ") " << std::endl;
            }
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

void MakeTrackedCut(TopoDS_Shape BaseShape, TopoDS_Shape CutShape, TDF_Label& LabelRoot){
    BRepAlgo_Cut MyCutter(BaseShape, CutShape);
    TopoDS_Shape ResultShape = MyCutter.Shape();

    TDF_Label Tool      = TDF_TagSource::NewChild(LabelRoot);
    TDF_Label Modified      = TDF_TagSource::NewChild(LabelRoot); 
    TDF_Label Deleted       = TDF_TagSource::NewChild(LabelRoot); 
    TDF_Label Intersections = TDF_TagSource::NewChild(LabelRoot); 
    TDF_Label NewFaces      = TDF_TagSource::NewChild(LabelRoot); 

    // push CUT results in DF as modification of Box1
    TNaming_Builder resultBuilder (LabelRoot);
    resultBuilder.Modify (BaseShape, ResultShape);

    // Select the 'cut shape', for some reason
    TNaming_Selector ToolSelector2(Tool);
    ToolSelector2.Select(CutShape, CutShape);

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

int main(){
    //runCase1();
    //runCase2();
    //runCase3();
    runCase4();
    return 0;
}
