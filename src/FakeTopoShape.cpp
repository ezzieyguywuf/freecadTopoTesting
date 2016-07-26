#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <TopoDS.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopExp.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopAbs_ShapeEnum.hxx>

#include "FakeTopoShape.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>

TopoShape::TopoShape(){
}

TopoShape::TopoShape(const TopoDS_Shape& sh) : _Shape(sh){
}

TopoShape::TopoShape(const TopoShape& sh) : _Shape(sh.getShape()){
}


TopoShape::~TopoShape(){
}

void TopoShape::operator = (const TopoShape& sh)
{
    //std::clog << "-----FakeTopoShape = operator" << std::endl;
    this->_Shape     = sh._Shape;
    this->_TopoNamer = sh._TopoNamer;
}

TopoDS_Shape TopoShape::getShape() const{
    return this->_Shape;
}

TopoNamingHelper TopoShape::getTopoHelper() const{
    return this->_TopoNamer;
}

void TopoShape::setShape(const TopoDS_Shape& shape){
    this->_Shape = shape;
}

void TopoShape::setShape(const TopoShape& shape){
    std::clog << "setShape(TopoShape) called" << std::endl;
    this->_Shape     = shape._Shape;
    this->_TopoNamer = shape._TopoNamer;
}

void TopoShape::createBox(const BoxData& BData){
    // Node 2
    this->_TopoNamer.AddNode("Tracked Shape");
    TopoData TData;
    BRepPrimAPI_MakeBox mkBox(BData.Length, BData.Width, BData.Height);
    // NOTE: The order here matters, as updateBox depends on it. That's why we use
    // `getBoxFacesVector` so that we always get the Faces in the same order.
    TData.NewShape      = mkBox.Shape();
    TData.GeneratedFaces = this->getBoxFacesVector(mkBox);
    this->_TopoNamer.TrackGeneratedShape("0:2", TData, "Generated Box Node");
    this->setShape(mkBox.Shape());
    //std::clog << "-----Dumpnig history after createBox" << std::endl;
    //std::clog << this->_TopoNamer.DeepDump();
}

void TopoShape::updateBox(const BoxData& BData){
    // TODO Do I need to check to ensure the Topo History is for a Box?
    //std::clog << "----dumping tree in updateBox" << std::endl;
    //std::clog << this->_TopoNamer.DeepDump() <<  std::endl;
    TopoData TData;
    BRepPrimAPI_MakeBox mkBox(BData.Length, BData.Width, BData.Height);
    TData.OldShape = this->getShape();
    TData.NewShape = mkBox.Shape();

    TopoDS_Face origFace, newFace;
    std::vector<TopoDS_Face> newFaces = this->getBoxFacesVector(mkBox);

    //std::clog << "-----Dumping history in update box" << std::endl;
    //std::clog << this->_TopoNamer.DeepDump();
    for (int i=0; i<=5; i++){
        // Tag 0:2:1:1:i+1 should hold the original face. GetLatestShape returns the latest
        // modification of this face in the Topological History
        std::ostringstream tag;
        tag << "0:2:1:1:" << (i+1);
        TopoDS_Shape origShape = _TopoNamer.GetLatestShape(tag.str());
        origFace = TopoDS::Face(origShape);
        newFace  = newFaces[i];

        if (!_TopoNamer.CompareTwoFaceTopologies(origFace, newFace)){
            TData.ModifiedFaces.push_back({origFace, newFace});
        }
    }
    //std::clog << "-----Dumping TopoHistory after update" << std::endl;
    //std::clog << _TopoNamer.DeepDump() << std::endl;

    this->_TopoNamer.TrackModifiedShape("0:2", TData, "Modified Box Node");
    this->setShape(mkBox.Shape());
}

void TopoShape::createFilletBaseShape(const TopoShape& BaseShape){
    // Node 2
    this->_TopoNamer.AddNode("BaseShapes");
    // Node 3
    this->_TopoNamer.AddNode("FilletShapes");

    // Collect and track appropriate data
    TopoData TData;

    TopTools_IndexedMapOfShape faces;
    // TODO need to handle possible seam edges
    TopExp::MapShapes(BaseShape.getShape(), TopAbs_FACE, faces);
    for (int i=1; i <= faces.Extent(); i++){
        TopoDS_Face face = TopoDS::Face(faces.FindKey(i));
        TData.GeneratedFaces.push_back(face);
    }

    TData.NewShape = BaseShape.getShape();
    this->_TopoNamer.TrackGeneratedShape(this->_TopoNamer.GetNode(2), TData, "Generated Base Shape");
    this->setShape(BaseShape.getShape());
}

BRepFilletAPI_MakeFillet TopoShape::createFillet(const TopoShape& BaseShape, const std::vector<FilletElement>& FDatas){
    // Make the fillets. NOTE: the edges should have already been 'selected' by
    // calling TopoShape::selectEdge(s) by the caller.
    BRepFilletAPI_MakeFillet mkFillet(BaseShape.getShape());

    for (auto&& FData: FDatas){
        TopoDS_Edge edge = this->_TopoNamer.GetSelectedEdge(FData.edgetag);
        mkFillet.Add(FData.radius1, FData.radius2, edge);
    }

    mkFillet.Build();

    FilletData TFData = this->getFilletData(BaseShape, mkFillet);
    
    this->_TopoNamer.TrackGeneratedShape(this->_TopoNamer.GetNode(3), TFData, "Filleted Shape");
    this->setShape(mkFillet.Shape());
    //std::clog << "-----Dumping topohistory after updateFillet" << std::endl;
    //std::clog << this->_TopoNamer.DeepDump() << std::endl;
    return mkFillet;
}

BRepFilletAPI_MakeFillet TopoShape::updateFillet(const TopoShape& BaseShape, const std::vector<FilletElement>& FDatas){
    // Update the BaseShape topo history as appropriate
    this->_TopoNamer.AppendTopoHistory("0:2", BaseShape.getTopoHelper());
    //std::clog << "-----Dumping after Append" << std::endl;
    //std::clog << this->_TopoNamer.DeepDump();
    //std::clog << this->_TopoNamer.DFDump();
 
    // Make the fillets. NOTE: the edges should have already been 'selected' by
    // calling TopoShape::selectEdge(s) by the caller.
    TopoDS_Shape localBaseShape = this->_TopoNamer.GetTipShape("0:2");
    BRepFilletAPI_MakeFillet mkFillet(localBaseShape);

    for (auto&& FData: FDatas){
        TopoDS_Edge edge = this->_TopoNamer.GetSelectedEdge(FData.edgetag);
        mkFillet.Add(FData.radius1, FData.radius2, edge);
    }

    mkFillet.Build();
    
    FilletData TFData = this->getFilletData(BaseShape, mkFillet);

    TFData.OldShape = this->getShape();
    TFData.NewShape = mkFillet.Shape();
    this->_TopoNamer.TrackModifiedShape(this->_TopoNamer.GetNode(3), TFData, "Filleted Shape");
    this->setShape(mkFillet.Shape());
    //std::clog << "-----Dumping topohistory after updateFillet" << std::endl;
    //std::clog << this->_TopoNamer.DeepDump() << std::endl;
    return mkFillet;
}

std::string TopoShape::selectEdge(const int edgeID){
    TopTools_IndexedMapOfShape listOfEdges;
    TopExp::MapShapes(_Shape, TopAbs_EDGE, listOfEdges);

    // Get the specific edge, I hope
    const TopoDS_Edge& anEdge = TopoDS::Edge(listOfEdges.FindKey(edgeID));

    std::string edgeLabel = this->_TopoNamer.SelectEdge(anEdge, _Shape);
    return edgeLabel;
}
//-------------------- Private Methods--------------------

std::vector<TopoDS_Face> TopoShape::getBoxFacesVector(BRepPrimAPI_MakeBox mkBox) const{
    std::vector<TopoDS_Face> OutFaces = {mkBox.TopFace(), mkBox.BottomFace(), mkBox.LeftFace(), mkBox.RightFace(), mkBox.FrontFace(), mkBox.BackFace()};
    return OutFaces;
}
TopTools_ListOfShape TopoShape::getBoxFaces(BRepPrimAPI_MakeBox mkBox) const{
    TopTools_ListOfShape OutFaces;
    OutFaces.Append(mkBox.TopFace());
    OutFaces.Append(mkBox.BottomFace());
    OutFaces.Append(mkBox.LeftFace());
    OutFaces.Append(mkBox.RightFace());
    OutFaces.Append(mkBox.FrontFace());
    OutFaces.Append(mkBox.BackFace());
    return OutFaces;
}

FilletData TopoShape::getFilletData(const TopoShape& BaseShape, BRepFilletAPI_MakeFillet& mkFillet) const{
    // Get the data we need for topo history
    FilletData TFData;

    TopTools_IndexedMapOfShape faces;
    // TODO need to handle possible seam edges
    // TODO need to pull BaseShape from topo tree
    TopExp::MapShapes(BaseShape.getShape(), TopAbs_FACE, faces);
    for (int i=1; i <= faces.Extent(); i++){
        TopoDS_Face face = TopoDS::Face(faces.FindKey(i));
        TopTools_ListOfShape modified = mkFillet.Modified(face);
        if (modified.Extent() == 1){
            TopoDS_Face newFace = TopoDS::Face(modified.First());
            TFData.ModifiedFaces.push_back({face, newFace});
        }
        else if (modified.Extent() != 0){
            throw std::runtime_error("Fillet should only produce a single modified face per face, or none");
        }
    }

    TopTools_IndexedMapOfShape edges;
    TopExp::MapShapes(BaseShape.getShape(), TopAbs_EDGE, edges);
    for (int i=1; i<=edges.Extent(); i++){
        TopoDS_Edge edge = TopoDS::Edge(edges.FindKey(i));
        TopTools_ListOfShape generated = mkFillet.Generated(edge);
        TopTools_ListIteratorOfListOfShape genIt(generated);
        for (; genIt.More(); genIt.Next()){
            TopoDS_Face genFace = TopoDS::Face(genIt.Value());
            TFData.GeneratedFacesFromEdge.push_back({edge, genFace});
        }
    }

    TopTools_IndexedMapOfShape vertexes;
    TopExp::MapShapes(BaseShape.getShape(), TopAbs_VERTEX, vertexes);
    for (int i=1; i<=vertexes.Extent(); i++){
        TopoDS_Vertex vertex = TopoDS::Vertex(vertexes.FindKey(i));
        TopTools_ListOfShape generated = mkFillet.Generated(vertex);
        TopTools_ListIteratorOfListOfShape genIt(generated);
        for (; genIt.More(); genIt.Next()){
            TopoDS_Face genFace = TopoDS::Face(genIt.Value());
            TFData.GeneratedFacesFromVertex.push_back({vertex, genFace});
        }
    }

    TFData.NewShape = mkFillet.Shape();
    return TFData;
}
