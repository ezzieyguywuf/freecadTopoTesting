#include <iostream>
#include <vector>

#include <TDataStd_AsciiString.hxx>
#include <TDF_IDFilter.hxx>
#include <TDF_TagSource.hxx>
#include <TDF_Tool.hxx>
#include <TDF_ChildIterator.hxx>

#include "TopoNamingHelper.h"

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>

#include <TopExp.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>

#include <TNaming_Builder.hxx>
#include <TNaming_Selector.hxx>
#include <TNaming_NamedShape.hxx>

TopoNamingHelper::TopoNamingHelper(){
    //std::clog << "-----Instantiated TopoNamingHelper\n";
    mySelectionNode = TDF_TagSource::NewChild(myRootNode);
    AddTextToLabel(mySelectionNode, "Selection Root Node");
}

TopoNamingHelper::TopoNamingHelper(const TopoNamingHelper& existing){
    std::clog << "-----Copying TopoNaming stuff\n";
    this->myDataFramework = existing.myDataFramework;
    this->myRootNode      = existing.myRootNode;
    this->mySelectionNode = existing.mySelectionNode;
}

TopoNamingHelper::~TopoNamingHelper(){
    //std::clog << "-----UnInstantiated TopoNamingHelper\n";
}

void TopoNamingHelper::operator = (const TopoNamingHelper& helper){
    std::clog << "-----Setting operator = TopoNaming stuff\n";
    this->myDataFramework = helper.myDataFramework;
    this->myRootNode      = helper.myRootNode;
    this->mySelectionNode = helper.mySelectionNode;
}

void TopoNamingHelper::TrackGeneratedShape(const TopoDS_Shape& GeneratedShape){
    std::clog << "-----Tracking Generated Shape\n";
    //std::ostringstream outputStream;
    //DeepDump(outputStream);
    //Base::Console().Message(outputStream.str().c_str());
    // Declare variables
    TNaming_Builder* MyBuilderPtr;
    TDF_Label curLabel;
    TopTools_IndexedMapOfShape mapOfFaces;
    TopoDS_Face curFace;
    int i;

    // create a new node under Root
    TDF_Label LabelRoot = TDF_TagSource::NewChild(myRootNode);

    AddTextToLabel(LabelRoot, "Generated Shape node");

    // add the generated shape to the LabelRoot
    MyBuilderPtr = new TNaming_Builder(LabelRoot);
    MyBuilderPtr->Generated(GeneratedShape);

    // Now iterate over each face and add as a child to the new node we created
    TopExp::MapShapes(GeneratedShape, TopAbs_FACE, mapOfFaces);
    for (i=1; i <= mapOfFaces.Extent(); i++){
        curFace = TopoDS::Face(mapOfFaces.FindKey(i));
        // Create a new sub-node for the Face
        curLabel = TDF_TagSource::NewChild(LabelRoot);
        // add Face to the sub-node
        delete MyBuilderPtr;
        MyBuilderPtr = new TNaming_Builder(curLabel);
        MyBuilderPtr->Generated(curFace);
    }
    //std::ostringstream outputStream2;
    //DeepDump(outputStream2);
    //std::clog << "Data Framework Dump Below\n";
    //Base::Console().Message(outputStream2.str().c_str());
}

void TopoNamingHelper::TrackFuseOperation(BRepAlgoAPI_Fuse& Fuser){
    std::clog << "-----Tracking Fuse Operation\n";
    // TODO: Need to update to account for an abritrary number of shapes being fused.
    // In a Fuse operation, each face is either Modified from one of the -two- (scratch
    // that) 'many' Shapes being fused, or it is Deleted. There is not an instance where a
    // Fuse operation results in a Generated Face.
    TopoDS_Shape BaseShape   = Fuser.Shape1();
    TopoDS_Shape FuseShape   = Fuser.Shape2();
    TopoDS_Shape ResultShape = Fuser.Shape();

    //TopTools_ListOfShape 

    // create two new nodes under Root. The first is to track the 'Shape1' as a
    // 'Generated' shape, and the second is to track the ResultShape, with  sub-nodes for
    // the modified faces on the BaseShape and the 'Shape1' as well. TODO: Need to figure
    // out a way to incorporate the topo tree from Shape1 into our topo tree, if Shape1
    // has one.
    std::clog << "Note: next 'tracking generated shape' msg from TrackFuseOperation\n";
    this->TrackGeneratedShape(FuseShape);
    TDF_Label LabelRoot    = TDF_TagSource::NewChild(myRootNode);
    TDF_Label BaseModified = TDF_TagSource::NewChild(LabelRoot);
    TDF_Label FuseModified = TDF_TagSource::NewChild(LabelRoot);

    // Add some descriptive text for debugging
    AddTextToLabel(LabelRoot, "Fusion Node");
    AddTextToLabel(BaseModified, "Modified Faces on Base Shape");
    AddTextToLabel(FuseModified, "Modified Faces on Add Shape");

    // Add the fused shape as a modification of BaseShape
    TNaming_Builder ResultBuilder(LabelRoot);
    ResultBuilder.Modify(BaseShape, ResultShape);

    // Add the BaseShape modified faces and the FuseShape modified faces
    TNaming_Builder BaseModifiedBuilder(BaseModified);
    TNaming_Builder FuseModifiedBuilder(FuseModified);

    TopTools_IndexedMapOfShape origFaces, fuseFaces, allBaseModified;

    TopExp::MapShapes(BaseShape, TopAbs_FACE, origFaces);
    for (int i=1; i<=origFaces.Extent(); i++){
        TopoDS_Face curFace = TopoDS::Face(origFaces.FindKey(i));
        TopTools_ListOfShape modified = Fuser.Modified(curFace);
        TopTools_ListIteratorOfListOfShape modIterator(modified);
        for (; modIterator.More(); modIterator.Next()){
            TopoDS_Face moddedFace = TopoDS::Face(modIterator.Value());
            if (!curFace.IsEqual(moddedFace)){
                BaseModifiedBuilder.Modify(curFace, moddedFace);
                allBaseModified.Add(moddedFace);
            }
        }
    }

    TopExp::MapShapes(FuseShape, TopAbs_FACE, fuseFaces);
    for (int i=1; i<=fuseFaces.Extent(); i++){
        TopoDS_Face curFace = TopoDS::Face(fuseFaces.FindKey(i));
        TopTools_ListOfShape modified = Fuser.Modified(curFace);
        TopTools_ListIteratorOfListOfShape modIterator(modified);
        for (; modIterator.More(); modIterator.Next()){
            TopoDS_Face moddedFace = TopoDS::Face(modIterator.Value());
            if (!curFace.IsEqual(moddedFace) && !allBaseModified.Contains(moddedFace)){
                FuseModifiedBuilder.Modify(curFace, moddedFace);
            }
        }
    }
    //std::ostringstream outputStream;
    //DeepDump(outputStream);
    //std::clog << "Data Framework Dump Below\n";
    //Base::Console().Message(outputStream.str().c_str());
}

void TopoNamingHelper::TrackFilletOperation(const TopoDS_Shape& BaseShape, BRepFilletAPI_MakeFillet& mkFillet){
    BRepFilletAPI_MakeFillet Filleter = mkFillet;
    std::clog << "-----Tracking Fillet Operation\n";
    //std::ostringstream output;
    //DeepDump(output);
    //Base::Console().Message(output.str().c_str());

    TopoDS_Shape ResultShape = Filleter.Shape();

    // Create a new node under the Root node for the result filleted Shape and it's
    // modified/deleted/generated Faces.
    TDF_Label FilletRoot        = TDF_TagSource::NewChild(myRootNode);
    TDF_Label Modified          = TDF_TagSource::NewChild(FilletRoot);
    TDF_Label Deleted           = TDF_TagSource::NewChild(FilletRoot);
    TDF_Label FacesFromEdges    = TDF_TagSource::NewChild(FilletRoot);
    TDF_Label FacesFromVertices = TDF_TagSource::NewChild(FilletRoot);

    // Add some descriptive text for debugging
    AddTextToLabel(FilletRoot, "Fillet Node");
    AddTextToLabel(Modified, "Modified faces");
    AddTextToLabel(Deleted, "Deleted faces");
    AddTextToLabel(FacesFromEdges, "Faces from edges");
    AddTextToLabel(FacesFromVertices, "Faces from vertices");

    // Start by adding the result shape. This will also create the TNaming_UsedShapes
    // under the Root node if it doesn't exist
    TNaming_Builder FilletBuilder(FilletRoot);
    FilletBuilder.Modify(BaseShape, ResultShape);

    // Next, the Faces generated from Edges
    TNaming_Builder FacesFromEdgeBuilder(FacesFromEdges);
    TopTools_IndexedMapOfShape mapOfEdges;
    TopExp::MapShapes(BaseShape, TopAbs_EDGE, mapOfEdges);
    for (int i=1; i<=mapOfEdges.Extent(); i++){
        TopoDS_Edge curEdge = TopoDS::Edge(mapOfEdges.FindKey(i));
        TopTools_ListOfShape generatedFaces = Filleter.Generated(curEdge);
        TopTools_ListIteratorOfListOfShape it(generatedFaces);
        for (;it.More(); it.Next()){
            TopoDS_Shape checkShape = it.Value();
            if (!curEdge.IsSame(checkShape)){
                FacesFromEdgeBuilder.Generated(curEdge, checkShape);
            }
        }
    }

    // Faces from BaseShape Modified or Deleted by the Fillet operation
    TNaming_Builder ModifiedBuilder(Modified);
    TNaming_Builder DeletedBuilder(Deleted);
    TopTools_IndexedMapOfShape mapOfFaces;
    TopExp::MapShapes(BaseShape, TopAbs_FACE, mapOfFaces);
    for (int i=1; i<=mapOfFaces.Extent(); i++){
        TopoDS_Face curFace = TopoDS::Face(mapOfFaces.FindKey(i));

        // First check Modified
        TopTools_ListOfShape modifiedFaces = Filleter.Modified(curFace);
        TopTools_ListIteratorOfListOfShape it(modifiedFaces);
        for (; it.More(); it.Next()){
            TopoDS_Shape checkShape = it.Value();
            if (!curFace.IsSame(checkShape)){
                ModifiedBuilder.Modify(curFace, checkShape);
            }
        }

        // Then check Deleted
        if (Filleter.IsDeleted(curFace)){
            DeletedBuilder.Delete(curFace);
        }
    }

    // Finally, the Faces generated from Vertices
    TNaming_Builder FacesFromVerticesBuilder(FacesFromVertices);
    TopTools_IndexedMapOfShape mapOfVertices;
    TopExp::MapShapes(BaseShape, TopAbs_VERTEX, mapOfVertices);
    for (int i=1; i<=mapOfVertices.Extent(); i++){
        TopoDS_Shape curVertex = mapOfVertices.FindKey(i);
        TopTools_ListOfShape generatedFaces = Filleter.Generated(curVertex);
        TopTools_ListIteratorOfListOfShape it(generatedFaces);
        for (;it.More(); it.Next()){
            TopoDS_Shape checkShape = it.Value();
            if (!curVertex.IsSame(checkShape)){
                FacesFromEdgeBuilder.Generated(curVertex, checkShape);
            }
        }
    }
    //std::ostringstream outputStream;
    //DeepDump(outputStream);
    //std::clog << "Data Framework Dump Below\n";
    //Base::Console().Message(outputStream.str().c_str());
}
std::string TopoNamingHelper::SelectEdge(const TopoDS_Edge anEdge, const TopoDS_Shape aShape){
    // The label returned will be for the selected Edge
    TDF_Label SelectedLabel;
    std::string SelecedLabelEntryString;
    // Whether or not the selected Edge is already in the tree
    bool found = false;
    // These will be used to check each existing Selection
    Handle(TNaming_NamedShape) EdgeNS;
    Handle(TNaming_NamedShape) ContextNS;
    TopoDS_Edge checkEdge;
    TopoDS_Shape checkShape;
    //std::clog << "Dumping whole tree before loop\n";
    //this->DeepDump();

    // Only loop through mySelectionNode if it has a child. You cannot rely on
    // TDF_ChildIterator to not loop through an empty branch.
    if (mySelectionNode.HasChild()){
        for (TDF_ChildIterator it(mySelectionNode, Standard_False); it.More(); it.Next()){
            TDF_Label EdgeNode    = it.Value();

            // Double check to make sure it was made using TNaming_Selector, i.e. that it
            // has a sub-node.
            if (EdgeNode.HasChild()){
                TDF_Label ContextNode = EdgeNode.FindChild(1, Standard_False);

                // Get the Edge and Contex Shape for this sub-node
                EdgeNode.FindAttribute(TNaming_NamedShape::GetID(), EdgeNS);
                ContextNode.FindAttribute(TNaming_NamedShape::GetID(), ContextNS);
                checkEdge  = TopoDS::Edge(EdgeNS->Get());
                checkShape = ContextNS->Get();

                // Check if this sub-node is the same as what's been passed in
                if (anEdge.IsEqual(checkEdge) && aShape.IsEqual(checkShape)){
                    SelectedLabel = EdgeNode;
                    found    = true;
                    break;
                }
            }
            else{
                std::runtime_error(
"ERROR! NODE SHOULD HAVE A SUB-NODE THAT CONTAINS THE CONTEXT SHAPE! Did you use TNaming_Selector or TNaming_Builder?");
            }
        }
    }

    // If not found, create.
    if (!found){
        SelectedLabel = TDF_TagSource::NewChild(mySelectionNode);
        //std::clog << "Dumping whole tree before select\n";
        //this->DeepDump();
        TNaming_Selector SelectionBuilder(SelectedLabel);
        SelectionBuilder.Select(anEdge, aShape);
        this->AddTextToLabel(SelectedLabel, "A selected edge. Sub-node is the context Shape");
        //std::clog << "Dumping whole tree after select\n";
        //this->DeepDump();
    }
    std::ostringstream dumpedEntry;
    SelectedLabel.EntryDump(dumpedEntry);
    return dumpedEntry.str();
}

std::vector<std::string> TopoNamingHelper::SelectEdges(const std::vector<TopoDS_Edge> Edges,
                                                     const TopoDS_Shape aShape){
    std::vector<std::string> outputLabels;
    for (std::vector<TopoDS_Edge>::const_iterator it = Edges.begin(); it != Edges.end(); ++it){
        TopoDS_Edge curEdge = *it;
        std::string curLabelEntry = this->SelectEdge(curEdge, aShape);
        outputLabels.push_back(curLabelEntry);
    }
    return outputLabels;
}

TopoDS_Edge TopoNamingHelper::GetSelectedEdge(const std::string NodeTag) const{
    TDF_Label EdgeNode;
    TDF_Tool::Label(myDataFramework, NodeTag.c_str(), EdgeNode);
    TopoDS_Edge SelectedEdge;
    if (!EdgeNode.IsNull()){
        Handle(TNaming_NamedShape) EdgeNS;
        EdgeNode.FindAttribute(TNaming_NamedShape::GetID(), EdgeNS);
        SelectedEdge = TopoDS::Edge(EdgeNS->Get());
    }
    else{
        std::runtime_error("That Node does not appear to exist on the Data Framework");
    }

    return SelectedEdge;
}

void TopoNamingHelper::AddTextToLabel(const TDF_Label& Label, char const *str){
    Handle(TDataStd_AsciiString) nameAttribute;
    TCollection_AsciiString myName;
    myName = str;
    nameAttribute = new TDataStd_AsciiString();
    nameAttribute->Set(myName);
    Label.AddAttribute(nameAttribute);
}

void TopoNamingHelper::Dump() const{
    TDF_Tool::DeepDump(std::cout, myDataFramework);
    std::cout << "\n";
}

void TopoNamingHelper::Dump(std::ostream& stream) const{
    TDF_Tool::DeepDump(stream, myDataFramework);
    stream << "\n";
}

void TopoNamingHelper::DeepDump(std::ostream& stream) const{
    TDF_IDFilter myFilter;
    myFilter.Keep(TDataStd_AsciiString::GetID());
    myFilter.Keep(TNaming_NamedShape::GetID());
    //TDF_Tool::ExtendedDeepDump(stream, myDataFramework, myFilter);
    //stream << "\n";
    TDF_ChildIterator TreeIterator(myRootNode, Standard_True);
    for(;TreeIterator.More(); TreeIterator.Next()){
        TDF_Label curLabel = TreeIterator.Value();
        // add the Tag info
        curLabel.EntryDump(stream);
        // If a AsciiString is present, add the data
        if (curLabel.IsAttribute(TDataStd_AsciiString::GetID())){
            Handle(TDataStd_AsciiString) date;
            curLabel.FindAttribute(TDataStd_AsciiString::GetID(), date);
            stream << " ";
            date->Get().Print(stream);
        }
        stream << "\n";
    }
}

void TopoNamingHelper::DeepDump() const{
    std::ostringstream output;
    DeepDump(output);
    std::cout << output.str();
}
