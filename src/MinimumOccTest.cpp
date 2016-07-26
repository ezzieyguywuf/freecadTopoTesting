#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>

#include <TopoNamingHelper.h>
#include "FakeTopoShape.h"

void TestMkFillet(){
    TopoDS_Shape Box = BRepPrimAPI_MakeBox(10., 10., 10.);
    BRepFilletAPI_MakeFillet mkFillet(Box);

    TopTools_IndexedMapOfShape edges;
    TopExp::MapShapes(Box, TopAbs_EDGE, edges);
    TopoDS_Edge edge = TopoDS::Edge(edges.FindKey(3));

    mkFillet.Add(1., 1., edge);
    mkFillet.Build();

    TopoDS_Shape result = mkFillet.Shape();


    TopTools_IndexedMapOfShape faces;
    TopExp::MapShapes(Box, TopAbs_FACE, faces);
    TopoDS_Face face = TopoDS::Face(faces.FindKey(1));

    for (int i=1; i<=faces.Extent(); i++){
        TopoDS_Face baseFace = TopoDS::Face(faces.FindKey(i));
        std::ostringstream name;
        name << "00_0"<< i << "_BaseFace";
        TopoNamingHelper::WriteShape(baseFace, name.str());

        const TopTools_ListOfShape& modified = mkFillet.Modified(baseFace);
        TopTools_ListIteratorOfListOfShape modIt(modified);
        for (int j=1; modIt.More(); modIt.Next(), j++){
            TopoDS_Face curFace = TopoDS::Face(modIt.Value());
            std::ostringstream name2;
            name2 << "00_0" << i << "_ModifiedFace";
            TopoNamingHelper::WriteShape(curFace, name2.str(), j);
        }
    }

    TopoNamingHelper::WriteShape(Box, "00_00_Box");
    TopoNamingHelper::WriteShape(result, "00_00_FilletResult");
    std::clog << "Done." << std::endl;
}

class Part{
    public:
        Part();
        ~Part();
        void setShape(const TopoShape& shape);
        const TopoShape& getShape() const;

    private:
        TopoShape _Shape;
};

Part::Part(){
}

Part::~Part(){
}

void Part::setShape(const TopoShape& shape){
    _Shape = shape;
}

const TopoShape& Part::getShape() const{
    return _Shape;
}


void TestResizeBox(){
    Part BoxPart;
    Part FilletPart;

    TopoShape BoxShape = BoxPart.getShape();

    // initial box
    std::clog << "------------------------------" << std::endl;
    std::clog << "Creating box" << std::endl;
    std::clog << "------------------------------" << std::endl;
    BoxData BData(10., 10., 10.);
    BoxShape.createBox(BData);
    //BoxShape._TopoNamer.DeepDump();
    BoxPart.setShape(BoxShape);

    // initial fillet
    std::clog << "------------------------------" << std::endl;
    std::clog << "Creating fillet" << std::endl;
    std::clog << "------------------------------" << std::endl;

    // First set the BaseShape
    TopoShape FilletShape = FilletPart.getShape();
    std::clog << "dumping" << std::endl;
    std::clog << FilletShape.getTopoHelper().DeepDump() << std::endl;
    FilletShape.createFilletBaseShape(BoxPart.getShape());

    // Next, gather the fillet data
    std::vector<FilletElement> FDatas;
    FilletElement FData;
    // Don't forget to select the appropriate Edge
    FData.edgetag = FilletShape.selectEdge(3);
    FData.edgeid  = 3;
    FData.radius1 = 1.;
    FData.radius2 = 1.;
    FDatas.push_back(FData);

    // finally, create the Fillet
    BRepFilletAPI_MakeFillet mkFillet = FilletShape.createFillet(BoxPart.getShape(), FDatas);
    FilletPart.setShape(FilletShape);

    //std::clog << "Dumping istory after fillet" << std::endl;
    //std::clog << FilletShape.getTopoHelper().DeepDump();

    //// taller box
    //std::clog << "------------------------------" << std::endl;
    //std::clog << "Updating box" << std::endl;
    //std::clog << "------------------------------" << std::endl;
    //BData.Height = 15.;
    //BoxShape.updateBox(BData);

    //std::clog << "Dumping history after updateBox" << std::endl;
    //std::clog << BoxShape.getTopoHelper().DeepDump();

    ////// rebuild fillet
    //std::clog << "------------------------------" << std::endl;
    //std::clog << "rebuilding fillet" << std::endl;
    //std::clog << "------------------------------" << std::endl;
    //FilletShape.updateFillet(BoxShape, FDatas);
    //std::clog << "Dumping history after updateFillet" << std::endl;
    //std::clog << FilletShape.getTopoHelper().DeepDump();
}

int main(){
    //TestMkFillet();
    TestResizeBox();
    return 0;
}
