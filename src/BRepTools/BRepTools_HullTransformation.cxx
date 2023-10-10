// Created on: 2023-07-07
// Created by: Irina KOCHETKOVA
// Copyright (c) 2023 OPEN CASCADE SAS
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


#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BRepTools_HullTransformation.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BezierSurface.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <TopLoc_Location.hxx>

IMPLEMENT_STANDARD_RTTIEXT(BRepTools_HullTransformation,BRepTools_Modification)

//=======================================================================
//function : BRepTools_HullTransformation
//purpose  : 
//=======================================================================
BRepTools_HullTransformation::BRepTools_HullTransformation()
{
}

//=======================================================================
//function : InitLinear
//purpose  : 
//=======================================================================
void BRepTools_HullTransformation::InitLinear(const double theCM,
                                              const double theCBNew,
                                              const double theCBOld,
                                              const double theLPP)
{
  myLinear = true;
  _cm = theCM;
  _cb_new = theCBNew;
  _cb_old = theCBOld;
  _lpp = theLPP;
  myScale = 1.0;// (_cm - _cb_new) / (_cm - _cb_old);
}

//=======================================================================
//function : InitQuad
//purpose  : 
//=======================================================================
void BRepTools_HullTransformation::InitQuad(const double theAftlim,
                                            const double theCCA,
                                            const double theCCF,
                                            const double theForelim,
                                            const double theAftCoef,
                                            const double theForeCoef,
                                            const bool theModifyAftZone,
                                            const bool theModifyForeZone)
{
  myLinear = false;

  _aftlim = theAftlim;
  _cca = theCCA;
  _ccf = theCCF;
  _forelim = theForelim;
  _aft_coef = theAftCoef;
  _fore_coef = theForeCoef;
  _modify_aft_zone = theModifyAftZone;
  _modify_fore_zone = theModifyForeZone;

  myScale = 1;
}


//=======================================================================
//function : NewPointFrom
//purpose  : 
//=======================================================================
gp_Pnt BRepTools_HullTransformation::NewPointFrom(const gp_Pnt& theOldPoint)
{
  Standard_Real oldX = theOldPoint.X();
  Standard_Real newX = _lpp / 2;
  Standard_Real incrX = 0;

  if (myLinear)
  {
    if (oldX <= _lpp / 2)
    {
      newX = oldX * (_cm - _cb_new) / (_cm - _cb_old);
      if (newX > _lpp / 2)
        newX = _lpp / 2;
    }
    else if (oldX > _lpp / 2)
    {
      newX = oldX * (_cm - _cb_new) / (_cm - _cb_old) +
        _lpp * (_cb_new - _cb_old) / (_cm - _cb_old);
      if (newX < _lpp / 2) {
        newX = _lpp / 2;
      }
    }
  }
  else
  {
    if (_modify_aft_zone &&          // Aft
        oldX >= _aftlim && oldX <= _cca)
    {
      incrX = _aft_coef * (oldX - _aftlim)*(oldX - _cca);
    }
    else if (_modify_fore_zone &&   // Fore
             oldX >= _ccf && oldX <= _forelim)
    {
      incrX = _fore_coef * (oldX - _ccf)*(oldX - _forelim);
    }
    newX = oldX + incrX;
  }
  return gp_Pnt(newX, theOldPoint.Y(), theOldPoint.Z());
}

//=======================================================================
//function : NewSurface
//purpose  : 
//=======================================================================
Standard_Boolean BRepTools_HullTransformation::NewSurface(const TopoDS_Face& F,
                                                          Handle(Geom_Surface)& S,
                                                          TopLoc_Location& L,
                                                          Standard_Real& Tol,
                                                          Standard_Boolean& RevWires,
                                                          Standard_Boolean& RevFace)
{
  S = Handle(Geom_Surface)::DownCast(BRep_Tool::Surface(F,L)->Copy());

  Tol = BRep_Tool::Tolerance(F);
  Tol *= myScale;
  RevWires = Standard_False;
  RevFace = (myScale > 0);
  S = Handle(Geom_Surface)::DownCast(S->Transformed(L.Transformation()));
  if (S.IsNull())
    return Standard_True;
  
  Handle(Standard_Type) TheTypeS = S->DynamicType();
  if (TheTypeS != STANDARD_TYPE(Geom_BSplineSurface))
  {
    throw Standard_NoSuchObject("BRepTools_HullTransformation : Not BSpline Type of Surface");
  }
  Handle(Geom_BSplineSurface) S2 = Handle(Geom_BSplineSurface)::DownCast(S);
  Standard_Real aUMin, aUMax, aVMin, aVMax;
  BRepTools::UVBounds(F, aUMin, aUMax, aVMin, aVMax);
  S2->CheckAndSegment(aUMin, aUMax, aVMin, aVMax);
  for (Standard_Integer i = 1; i <= S2->NbUPoles(); i++)
  {
    for (Standard_Integer j = 1; j <= S2->NbVPoles(); j++) {
      gp_Pnt P = NewPointFrom(S2->Pole(i, j));
      S2->SetPole(i, j, P);
    }
  }

  L.Identity();
  return Standard_True;
}

//=======================================================================
//function : NewCurve
//purpose  : 
//=======================================================================
Standard_Boolean BRepTools_HullTransformation::NewCurve(const TopoDS_Edge& E,
                                                        Handle(Geom_Curve)& C,
                                                        TopLoc_Location& L,
                                                        Standard_Real& Tol)
{
  Standard_Real f,l;
  Tol = BRep_Tool::Tolerance(E) * myScale;
  C = BRep_Tool::Curve(E, L, f, l);
  if (C.IsNull())
    return Standard_True;
  
  C = Handle(Geom_Curve)::DownCast(C->Copy()->Transformed(L.Transformation()));
  Handle(Standard_Type) TheTypeC = C->DynamicType();
  if (TheTypeC != STANDARD_TYPE(Geom_BSplineCurve))
  {
    throw Standard_NoSuchObject("BRepTools_HullTransformation : Not BSpline Type of Curve");
  }

  Handle(Geom_BSplineCurve) C2 = Handle(Geom_BSplineCurve)::DownCast(C);
  C2->Segment(f, l);
  for(Standard_Integer i = 1; i <= C2->NbPoles(); i++)
  {
    gp_Pnt P = NewPointFrom(C2->Pole(i));
    C2->SetPole(i, P);
  }

  L.Identity();
  return Standard_True;
}

//=======================================================================
//function : NewPoint
//purpose  : 
//=======================================================================
Standard_Boolean BRepTools_HullTransformation::NewPoint(const TopoDS_Vertex& V,
                                                        gp_Pnt& P,
                                                        Standard_Real& Tol)
{
  gp_Pnt Pnt = BRep_Tool::Pnt(V);
  Tol = BRep_Tool::Tolerance(V);
  Tol *= myScale;
  P = NewPointFrom(Pnt);

  return Standard_True;
}

//=======================================================================
//function : NewCurve2d
//purpose  : 
//=======================================================================
Standard_Boolean BRepTools_HullTransformation::NewCurve2d(const TopoDS_Edge&,
                                                          const TopoDS_Face&,
                                                          const TopoDS_Edge&,
                                                          const TopoDS_Face&,
                                                          Handle(Geom2d_Curve)&,
                                                          Standard_Real& )
{
  return Standard_False;
}

//=======================================================================
//function : NewParameter
//purpose  : 
//=======================================================================
Standard_Boolean BRepTools_HullTransformation::NewParameter(const TopoDS_Vertex&,
                                                            const TopoDS_Edge&,
                                                            Standard_Real&,
                                                            Standard_Real&)
{
  return Standard_False;
}

//=======================================================================
//function : Continuity
//purpose  : 
//=======================================================================
GeomAbs_Shape BRepTools_HullTransformation::Continuity(const TopoDS_Edge& E,
                                                       const TopoDS_Face& F1,
                                                       const TopoDS_Face& F2,
                                                       const TopoDS_Edge&,
                                                       const TopoDS_Face&,
                                                       const TopoDS_Face&)
{
  return BRep_Tool::Continuity(E,F1,F2);
}
