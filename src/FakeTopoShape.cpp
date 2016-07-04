#include "FakeTopoShape.h"
#include "FakeTopoShapeRunCases.h"
#include <BRepBuilderAPI_Copy.hxx>
#include <BRepTools.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <iostream>

// Constructors
TopoShape::TopoShape(){
}

TopoShape::~TopoShape(){
}

TopoShape::TopoShape(const TopoDS_Shape& shape)
    : _Shape(shape)
{
    _TopoNamer.TrackGeneratedShape(shape);
}
TopoShape::TopoShape(const TopoShape& shape)
    : _Shape(shape._Shape)
{
    _TopoNamer = shape._TopoNamer;
}

// copy thing
void TopoShape::operator = (const TopoShape& shape){
    _Shape     = shape._Shape;
    _TopoNamer = shape._TopoNamer;
}

// Topo Naming Setters
void TopoShape::setShape(const TopoDS_Shape& shape, const std::string& name){
    this->_Shape = shape;
    std::clog << "-----NOTE! Evolution = brandnew?! Name = " << name <<"\n";
    // Since the TopoDS_Shape can't carry a history with it, start the TNaming all over.
    // TODO: Check to make sure this isn't used incorrectly?
    TopoNamingHelper newTopoNamer;
    this->_TopoNamer = newTopoNamer;
    this->_TopoNamer.TrackGeneratedShape(shape, name);
}

void TopoShape::addShape(const TopoShape& shape){
    this->_Shape = shape._Shape;
    this->_TopoNamer.TrackGeneratedShape(shape._Shape);
}
void TopoShape::modifyShape(const std::string& NodeTag, const TopoDS_Shape& shape){
    this->_TopoNamer.TrackModifiedShape(NodeTag, shape);
}

TopoDS_Shape TopoShape::getShape() const{
    return _Shape;
}

void TopoShape::setShape(const TopoShape& shape){
    _Shape     = shape._Shape;
    _TopoNamer = shape._TopoNamer;
}
void TopoShape::setShape(const TopoShape& Shape, BRepAlgoAPI_Fuse& mkFuse){
    TopoDS_Shape resShape = mkFuse.Shape();
    if (!this->_Shape.IsEqual(resShape)){
        // First, copy the _TopoNamer from the passed shape
        this->_TopoNamer = Shape._TopoNamer;
        // Now, store the result shape
        this->_Shape = resShape;
        // Finally, track the new fused shape in the Topo Tree.
        this->_TopoNamer.TrackFuseOperation(mkFuse);
    }
}

void TopoShape::setShape(const TopoShape& BaseShape, BRepFilletAPI_MakeFillet& mkFillet){
    TopoDS_Shape resShape = mkFillet.Shape();
    if (!this->_Shape.IsEqual(resShape)){
        // First, copy the _TopoNamer from the passed shape
        this->_TopoNamer = BaseShape._TopoNamer;
        // Now, store the result shape
        this->_Shape = resShape;
        // Finally, track the new filleted shape in the Topo Tree.
        this->_TopoNamer.TrackFilletOperation(BaseShape._Shape, mkFillet);
    }
}

std::string TopoShape::selectEdge(const int edgeID){
    TopTools_IndexedMapOfShape listOfEdges;
    TopExp::MapShapes(_Shape, TopAbs_EDGE, listOfEdges);

    // Get the specific edge, I hope
    const TopoDS_Edge& anEdge = TopoDS::Edge(listOfEdges.FindKey(edgeID));

    std::string edgeLabel = this->_TopoNamer.SelectEdge(anEdge, _Shape);
    return edgeLabel;
}

std::vector<std::string> TopoShape::selectEdges(const std::vector<int> edgeIDs){
    std::vector<TopoDS_Edge> Edges;

    TopTools_IndexedMapOfShape listOfEdges;
    TopExp::MapShapes(_Shape, TopAbs_EDGE, listOfEdges);
    TopoDS_Edge anEdge;
    for (std::vector<int>::const_iterator it = edgeIDs.begin(); it != edgeIDs.end(); ++it){
        int curID = *it;
        anEdge = TopoDS::Edge(listOfEdges.FindKey(curID));
        Edges.push_back(anEdge);
    }
    std::vector<std::string> edgeLabels = this->_TopoNamer.SelectEdges(Edges, _Shape);
    return edgeLabels;
}

// TopoNaming getters
TopoDS_Edge TopoShape::getSelectedEdge(const std::string NodeTag) const{
    TopoDS_Edge selectedEdge = this->_TopoNamer.GetSelectedEdge(NodeTag);
    return selectedEdge;
}

TopoDS_Shape TopoShape::getSelectedBaseShape(const std::string NodeTag) const{
    TopoDS_Shape baseShape = this->_TopoNamer.GetSelectedBaseShape(NodeTag);
    return baseShape;
}

TopoDS_Shape TopoShape::getNodeShape(const std::string NodeTag) const{
    TopoDS_Shape nodeShape = this->_TopoNamer.GetNodeShape(NodeTag);
    return nodeShape;
}

TopoDS_Shape TopoShape::getTipShape(){
    TopoDS_Shape tipShape = this->_TopoNamer.GetTipShape();
    //if (tipShape.IsNull()) {
        //std::clog << "-----tipShape is NULL (in FakeTopoSahpe)...\n";
    //}
    //else {
        //std::clog << "-----tipshape is not NULL (in FakeTopoShape)!!!\n";
    //}
    return tipShape;
}

std::string TopoShape::DumpTopoHistory() const{
    return this->_TopoNamer.DeepDump();
}

std::string TopoShape::DeepDumpTopoHistory() const{
    std::ostringstream outStream;
    this->_TopoNamer.Dump(outStream);
    return outStream.str();
}

std::string TopoShape::DeepDeepDumpTopoHistory() const{
    std::ostringstream outStream;
    outStream << this->_TopoNamer.DFDump();
    return outStream.str();
}

void TopoShape::OCCDeepDump() const{
    this->_TopoNamer.Dump();
}

void TopoShape::WriteTNamingNode(const std::string NodeTage, const std::string NameBase, const bool Deep = true){
    this->_TopoNamer.WriteNode(NodeTage, NameBase, Deep);
}

BRepFilletAPI_MakeFillet TopoShape::makeTopoShapeFillet(const double rad1, const double rad2, const std::string which){
    //Base::Console().Message("-----makeTopoShapeFillet (TopoShape) called, dumpinghistory\n");
    //Base::Console().Message(this->DumpTopoHistory().c_str());
    TopoDS_Shape baseShape = this->getTipShape();
    TopoDS_Shape testShape = this->getSelectedBaseShape(which);

    //if (baseShape.IsNull()){
        //std::clog << "-----baseShape is NULL (in FakeTopoShape)..." << std::endl;
    //}
    //else{
        //std::clog << "-----baseShape is NOT NULL (in FakeTopoShape)!!!" << std::endl;
    //}

    BRepFilletAPI_MakeFillet mkFillet(baseShape);

    std::clog << "-----Retrieving edge for NodeTag=" << which << "\n";
    TopoDS_Edge edge    = TopoDS::Edge(this->getSelectedEdge(which));

    //if (edge.IsNull()){
        //std::clog << "-----edge is NULL (in FakeTopoShape.cpp)..." << std::endl;
    //}
    //else{
        //std::clog << "-----edge is NOT NULL (in FakeTopoShape.cpp)!!!" << std::endl;
    //}

    TopTools_IndexedMapOfShape edges;
    TopExp::MapShapes(baseShape, TopAbs_EDGE, edges);
    //if (edges.Contains(edge)){
        //std::cout << "-----Yes the Edge is in the box...\n";
    //}
    //else{
        //std::cout << "-----No, the Edge is not in the Box...\n";
    //}

    mkFillet.Add(rad1, rad2, edge);
    mkFillet.Build();
    this->_TopoNamer.TrackFilletOperation(baseShape, mkFillet);
    this->_Shape = mkFillet.Shape();
    return mkFillet;
}


// Other stuff from Real TopoShape
TopoDS_Shape TopoShape::fuse(TopoDS_Shape shape) const
{
    std::clog << "-----TopoShape::fuse called\n";
    if (this->_Shape.IsNull())
        Standard_Failure::Raise("Base shape is null");
    if (shape.IsNull())
        Standard_Failure::Raise("Tool shape is null");
    BRepAlgoAPI_Fuse mkFuse(this->_Shape, shape);
    //_TopoNamer.TrackFuseOperation(mkFuse);
    return mkFuse.Shape();
}

TopoDS_Shape TopoShape::multiFuse(const std::vector<TopoDS_Shape>& shapes, Standard_Real tolerance) const
{
    std::clog << "-----TopoShape::multiFuse called\n";
    if (this->_Shape.IsNull())
        Standard_Failure::Raise("Base shape is null");
    BRepAlgoAPI_Fuse mkFuse;
    TopTools_ListOfShape shapeArguments,shapeTools;
    shapeArguments.Append(this->_Shape);
    for (std::vector<TopoDS_Shape>::const_iterator it = shapes.begin(); it != shapes.end(); ++it) {
        if (it->IsNull()){
            std::clog << "!!!ERROR!!! Tool shape is null\n!";
            return mkFuse.Shape();
        }
        if (tolerance > 0.0)
            // workaround for http://dev.opencascade.org/index.php?q=node/1056#comment-520
            shapeTools.Append(BRepBuilderAPI_Copy(*it).Shape());
        else
            shapeTools.Append(*it);
    }
    mkFuse.SetArguments(shapeArguments);
    mkFuse.SetTools(shapeTools);
    if (tolerance > 0.0)
        mkFuse.SetFuzzyValue(tolerance);
    mkFuse.Build();
    if (!mkFuse.IsDone()){
        std::clog << "!!!ERROR!!! MultiFusion failed\n";
        return mkFuse.Shape();
    }
    TopoDS_Shape resShape = mkFuse.Shape();
    return resShape;
}

// Main func
int main(){
    try{
        //TopoShape res = DuplicateCylinderFilletBug();
        //std::clog << "--------------------------------------------------" << std::endl;
        TopoShape res = SimpleBoxWithNaming();
        res.DumpTopoHistory();
    }
    catch(Standard_Failure failure){
        failure.Print(std::clog);
        std::clog << std::endl;
        failure.Reraise();
    }
    return 0;
}
