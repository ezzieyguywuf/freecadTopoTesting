#ifndef __TOPOSHAPE__
#define __TOPOSHAPE__
#include <BRepAlgoAPI_Fuse.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <vector>
#include <string>

#include "TopoNamingHelper.h"

class TopoShape{
    public:
        TopoShape();
        TopoShape(const TopoDS_Shape& shape);
        TopoShape(const TopoShape& shape);
        ~TopoShape();

        void operator = (const TopoShape& shape);

        // Added for Topo Naming stuff
        void setShape(const TopoDS_Shape& shape, const std::string& name="");
        TopoDS_Shape getShape() const;
        void addShape(const TopoShape& shape);
        void modifyShape(const std::string& NodeTag, const TopoDS_Shape& shape);
        void setShape(const TopoShape& shape);
        void setShape(const TopoShape& Shape, BRepAlgoAPI_Fuse& mkFuse);
        void setShape(const TopoShape& BaseShape, BRepFilletAPI_MakeFillet& mkFillet);
        BRepFilletAPI_MakeFillet makeTopoShapeFillet(const double rad1, const double rad2, const std::string which);
        std::string DumpTopoHistory() const;
        std::string DeepDumpTopoHistory() const;
        std::string DeepDeepDumpTopoHistory() const;
        void OCCDeepDump() const;

        std::string selectEdge(const int edgeID);
        std::vector<std::string> selectEdges(const std::vector<int> edgeIDs);
        TopoDS_Edge getSelectedEdge(const std::string NodeTag) const;
        TopoDS_Shape getSelectedBaseShape(const std::string NodeTag) const;
        TopoDS_Shape getTipShape();
        TopoDS_Shape getNodeShape(const std::string NodeTag) const;

        void WriteTNamingNode(const std::string NodeTage, const std::string NameBase, const bool Deep);

        // Other stuff from Real TopoShape
        TopoDS_Shape fuse(TopoDS_Shape) const;
        TopoDS_Shape multiFuse(const std::vector<TopoDS_Shape>&, Standard_Real tolerance = 0.0) const;
    private:
        TopoDS_Shape _Shape;
        TopoNamingHelper _TopoNamer;
};
#endif //__TOPOSHAPE__
