// Copyright (c) 1999-2014 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#include <TransferBRep_ShapeBinder.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_CompSolid.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TransferBRep_ShapeInfo.hxx>

IMPLEMENT_STANDARD_RTTIEXT(TransferBRep_ShapeBinder, Transfer_Binder)

//=======================================================================
//function : Value
//purpose  :
//=======================================================================
TransferBRep_ShapeBinder::TransferBRep_ShapeBinder() {}

//=======================================================================
//function : Value
//purpose  :
//=======================================================================
TransferBRep_ShapeBinder::TransferBRep_ShapeBinder(const TopoDS_Shape& shape)
  : theres(shape)
{
  SetResultPresent();
}

//=======================================================================
//function : Value
//purpose  :
//=======================================================================
TopAbs_ShapeEnum  TransferBRep_ShapeBinder::ShapeType() const
{
  if (!HasResult()) return TopAbs_SHAPE;
  return  Result().ShapeType();
}

//=======================================================================
//function : Value
//purpose  :
//=======================================================================
TopoDS_Vertex  TransferBRep_ShapeBinder::Vertex() const
{
  return TopoDS::Vertex(Result());
}

//=======================================================================
//function : Value
//purpose  :
//=======================================================================
TopoDS_Edge  TransferBRep_ShapeBinder::Edge() const
{
  return TopoDS::Edge(Result());
}

//=======================================================================
//function : Value
//purpose  :
//=======================================================================
TopoDS_Wire  TransferBRep_ShapeBinder::Wire() const
{
  return TopoDS::Wire(Result());
}

//=======================================================================
//function : Value
//purpose  :
//=======================================================================
TopoDS_Face  TransferBRep_ShapeBinder::Face() const
{
  return TopoDS::Face(Result());
}

//=======================================================================
//function : Value
//purpose  :
//=======================================================================
TopoDS_Shell  TransferBRep_ShapeBinder::Shell() const
{
  return TopoDS::Shell(Result());
}

//=======================================================================
//function : Value
//purpose  :
//=======================================================================
TopoDS_Solid  TransferBRep_ShapeBinder::Solid() const
{
  return TopoDS::Solid(Result());
}

//=======================================================================
//function : Value
//purpose  :
//=======================================================================
TopoDS_CompSolid  TransferBRep_ShapeBinder::CompSolid() const
{
  return TopoDS::CompSolid(Result());
}

//=======================================================================
//function : Value
//purpose  :
//=======================================================================
TopoDS_Compound  TransferBRep_ShapeBinder::Compound() const
{
  return TopoDS::Compound(Result());
}

//=======================================================================
//function : Value
//purpose  :
//=======================================================================
Handle(Standard_Type)  TransferBRep_ShapeBinder::ResultType() const
{
  return  TransferBRep_ShapeInfo::Type(theres);
}

//=======================================================================
//function : Value
//purpose  :
//=======================================================================
Standard_CString  TransferBRep_ShapeBinder::ResultTypeName() const
{
  return  TransferBRep_ShapeInfo::TypeName(theres);
} 

//=======================================================================
//function : Value
//purpose  :
//=======================================================================
void  TransferBRep_ShapeBinder::SetResult(const TopoDS_Shape& res)
{
  SetResultPresent();
  theres = res;
}

//=======================================================================
//function : Value
//purpose  :
//=======================================================================
const TopoDS_Shape& TransferBRep_ShapeBinder::Result() const
{
  return theres;
}

//=======================================================================
//function : Value
//purpose  :
//=======================================================================
TopoDS_Shape& TransferBRep_ShapeBinder::CResult()
{
  SetResultPresent(); return theres;
}