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
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.        *
******************************************************************************** */
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
