// Created on: 1994-02-24
// Created by: Laurent BOURESCHE
// Copyright (c) 1994-1999 Matra Datavision
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


#include <BRepAdaptor_Surface.hxx>
#include <BRepLProp_SurfaceTool.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

//=======================================================================
//function : Value
//purpose  : 
//=======================================================================
void BRepLProp_SurfaceTool::Value(const Handle(BRepAdaptor_Surface)& S, 
				  const Standard_Real U, 
				  const Standard_Real V, 
				  gp_Pnt& P)
{
  Standard_ASSERT_RAISE(!S.IsNull(), "Surface Adaptor is not initialized");
  P = S->Value(U, V);
}


//=======================================================================
//function : D1
//purpose  : 
//=======================================================================

void BRepLProp_SurfaceTool::D1(const Handle(BRepAdaptor_Surface)& S, 
			       const Standard_Real U, 
			       const Standard_Real V, 
			       gp_Pnt& P, 
			       gp_Vec& D1U, 
			       gp_Vec& D1V)
{
  Standard_ASSERT_RAISE(!S.IsNull(), "Surface Adaptor is not initialized");
  S->D1(U, V, P, D1U, D1V);
}


//=======================================================================
//function : D2
//purpose  : 
//=======================================================================

void BRepLProp_SurfaceTool::D2(const Handle(BRepAdaptor_Surface)& S, 
			       const Standard_Real U, 
			       const Standard_Real V, 
			       gp_Pnt& P, 
			       gp_Vec& D1U, 
			       gp_Vec& D1V, 
			       gp_Vec& D2U, 
			       gp_Vec& D2V, 
			       gp_Vec& DUV)
{
  Standard_ASSERT_RAISE(!S.IsNull(), "Surface Adaptor is not initialized");
  S->D2(U, V, P, D1U, D1V, D2U, D2V, DUV);
}

//=======================================================================
//function : DN
//purpose  : 
//=======================================================================
gp_Vec BRepLProp_SurfaceTool::DN(const Handle(BRepAdaptor_Surface)& S, 
				 const Standard_Real U, 
				 const Standard_Real V,
				 const Standard_Integer IU,
				 const Standard_Integer IV)
{
  Standard_ASSERT_RAISE(!S.IsNull(), "Surface Adaptor is not initialized");
  return S->DN(U, V, IU, IV);
}


//=======================================================================
//function : Continuity
//purpose  : 
//=======================================================================

Standard_Integer BRepLProp_SurfaceTool::Continuity
  (const Handle(BRepAdaptor_Surface)& S)
{
  Standard_ASSERT_RAISE(!S.IsNull(), "Surface Adaptor is not initialized");
  GeomAbs_Shape s = (GeomAbs_Shape) Min(S->UContinuity(),S->VContinuity());
  switch (s) {
  case GeomAbs_C0:
    return 0;
  case GeomAbs_C1:
    return 1;
  case GeomAbs_C2:
    return 2;
  case GeomAbs_C3:
    return 3;
  case GeomAbs_G1:
    return 0;
  case GeomAbs_G2:
    return 0;
  case GeomAbs_CN:
    return 3;
  };
  return 0;
}


//=======================================================================
//function : Bounds
//purpose  : 
//=======================================================================

void BRepLProp_SurfaceTool::Bounds(const Handle(BRepAdaptor_Surface)& S, 
				   Standard_Real& U1, 
				   Standard_Real& V1, 
				   Standard_Real& U2, 
				   Standard_Real& V2)
{
  Standard_ASSERT_RAISE(!S.IsNull(), "Surface Adaptor is not initialized");
  U1 = S->FirstUParameter();
  V1 = S->FirstVParameter();
  U2 = S->LastUParameter();
  V2 = S->LastVParameter();
}








