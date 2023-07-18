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


#include <BRepBuilderAPI_HullTransform.hxx>
#include <BRepBuilderAPI_NurbsConvert.hxx>
#include <BRepTools_HullTransformation.hxx>

//=======================================================================
//function : BRepBuilderAPI_HullTransform
//purpose  : 
//=======================================================================
BRepBuilderAPI_HullTransform::BRepBuilderAPI_HullTransform ()
{
  myModification = new BRepTools_HullTransformation();
}

//=======================================================================
//function : InitLinear
//purpose  : 
//=======================================================================
void BRepBuilderAPI_HullTransform::InitLinear(const double theCM,
                                              const double theCBNew,
                                              const double theCBOld,
                                              const double theLPP)
{
  Handle(BRepTools_HullTransformation) aHullTrsf = Handle(BRepTools_HullTransformation)::DownCast(myModification);
  aHullTrsf->InitLinear(theCM, theCBNew, theCBOld, theLPP);
}

//=======================================================================
//function : InitQuad
//purpose  : 
//=======================================================================
void BRepBuilderAPI_HullTransform::InitQuad(const double theAftlim,
                                            const double theCCA,
                                            const double theCCF,
                                            const double theForelim,
                                            const double theAftCoef,
                                            const double theForeCoef,
                                            const bool theModifyAftZone,
                                            const bool theModifyForeZone)
{
  Handle(BRepTools_HullTransformation) aHullTrsf = Handle(BRepTools_HullTransformation)::DownCast(myModification);
  aHullTrsf->InitQuad(theAftlim, theCCA, theCCF, theForelim, theAftCoef, theForeCoef, theModifyAftZone, theModifyForeZone);
}


//=======================================================================
//function : Perform
//purpose  : 
//=======================================================================

void BRepBuilderAPI_HullTransform::Perform(const TopoDS_Shape& S,
                                           const Standard_Boolean Copy)
{
  BRepBuilderAPI_NurbsConvert nc;
  nc.Perform(S, Copy);
  myHist.Add(S,nc);
  TopoDS_Shape Slocal = nc.Shape();
  Handle(BRepTools_HullTransformation) theModif =
    Handle(BRepTools_HullTransformation)::DownCast(myModification);
  DoModif(Slocal,myModification);
}


//=======================================================================
//function : Modified
//purpose  : 
//=======================================================================

const TopTools_ListOfShape& BRepBuilderAPI_HullTransform::Modified
  (const TopoDS_Shape& F)
{
  myGenerated.Clear();
  const TopTools_DataMapOfShapeListOfShape& M = myHist.Modification();
  if (M.IsBound(F)) { 
    TopTools_ListOfShape Li;
    TopTools_ListIteratorOfListOfShape itL(M(F));
    for (;itL.More();itL.Next())
      Li.Assign(BRepBuilderAPI_ModifyShape::Modified(itL.Value()));
  }
  return myGenerated;
}


//=======================================================================
//function : ModifiedShape
//purpose  : 
//=======================================================================

TopoDS_Shape BRepBuilderAPI_HullTransform::ModifiedShape
  (const TopoDS_Shape& S) const
{
  const TopTools_DataMapOfShapeListOfShape &aMapModif = myHist.Modification();
  TopoDS_Shape                              aShape    = S;

  if (aMapModif.IsBound(S)) {
    const TopTools_ListOfShape &aListModShape = aMapModif(S);
    Standard_Integer            aNbShapes     = aListModShape.Extent();

    if (aNbShapes > 0)
      aShape = aListModShape.First();
  }

  return BRepBuilderAPI_ModifyShape::ModifiedShape(aShape);
}

