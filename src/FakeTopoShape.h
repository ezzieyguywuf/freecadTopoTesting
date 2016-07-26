#ifndef FAKE_TOPO_SHAPE_H
#define FAKE_TOPO_SHAPE_H
#include <TopoDS_Shape.hxx>
#include <TopoNamingHelper.h>
#include <TopoNamingData.h>

struct FilletElement {
    FilletElement(){}
    FilletElement(int id, double rad1, double rad2){
        edgeid = id;
        radius1 = rad1;
        radius2 = rad2;
    }
    int edgeid;
    double radius1, radius2;
    std::string edgetag;
};

class TopoShape{
    public:
        TopoShape();
        TopoShape(const TopoDS_Shape& sh);
        TopoShape(const TopoShape& sh);
        ~TopoShape();

        void operator = (const TopoShape& sh);

        void createBox(const BoxData& BData);
        void updateBox(const BoxData& BData);
        void createFilletBaseShape(const TopoShape& BaseShape);
        BRepFilletAPI_MakeFillet createFillet(const TopoShape& BaseShape, const std::vector<FilletElement>& FDatas);
        BRepFilletAPI_MakeFillet updateFillet(const TopoShape& BaseShape, const std::vector<FilletElement>& FDatas);

        void setShape(const TopoDS_Shape& shape);
        void setShape(const TopoShape& shape);
        TopoDS_Shape getShape() const;
        TopoNamingHelper getTopoHelper() const;
        std::string selectEdge(const int edgeID);

    //private:
        TopoNamingHelper _TopoNamer;
        TopoDS_Shape _Shape;
        std::vector<TopoDS_Face> getBoxFacesVector(BRepPrimAPI_MakeBox mkBox) const;
        TopTools_ListOfShape getBoxFaces(BRepPrimAPI_MakeBox mkBox) const;
        FilletData getFilletData(const TopoShape& BaseShape, BRepFilletAPI_MakeFillet& mkFillet) const;
};
#endif /* ifndef FAKE_TOPO_SHAPE_H */
