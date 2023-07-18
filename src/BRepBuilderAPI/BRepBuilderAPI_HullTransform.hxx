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

#ifndef _BRepBuilderAPI_HullTransform_HeaderFile
#define _BRepBuilderAPI_HullTransform_HeaderFile

#include <BRepBuilderAPI_Collect.hxx>
#include <BRepBuilderAPI_ModifyShape.hxx>

//! Custom geometric transformation on a shape.
class BRepBuilderAPI_HullTransform  : public BRepBuilderAPI_ModifyShape
{
public:

  DEFINE_STANDARD_ALLOC

  
  //! Constructs a framework for applying the geometric
  //! transformation to a shape.
  //! Use InitLinear/InitQuad to define transformation parameters.
  //! After that use Perform to define the shape to transform and do the modification.
  Standard_EXPORT BRepBuilderAPI_HullTransform();

  //! Initialize parameters for linear transformation function.
  Standard_EXPORT void InitLinear(const double theCM,
                                  const double theCBNew,
                                  const double theCBOld,
                                  const double theLPP);

  //! Initialize parameters for quad transformation function.
  Standard_EXPORT void InitQuad(const double theAftlim,
                                const double theCCA,
                                const double theCCF,
                                const double theForelim,
                                const double theAftCoef,
                                const double theForeCoef,
                                const bool theModifyAftZone,
                                const bool theModifyForeZone);
  
  //! Applies the geometric transformation defined at the
  //! time of construction of this framework to the shape S.
  //! The transformation will applied to a duplicate of S.
  //! Use the function Shape to access the result.
  //! Note: this framework can be reused to apply the same
  //! geometric transformation to other shapes: just specify
  //! them by calling the function Perform again.
  Standard_EXPORT void Perform (const TopoDS_Shape& S, const Standard_Boolean Copy = Standard_True);
  
  //! Returns the list  of shapes modified from the shape
  //! <S>.
  Standard_EXPORT virtual const TopTools_ListOfShape& Modified (const TopoDS_Shape& S) Standard_OVERRIDE;
  
  //! Returns the modified shape corresponding to <S>.
  Standard_EXPORT virtual TopoDS_Shape ModifiedShape (const TopoDS_Shape& S) const Standard_OVERRIDE;

private:

  BRepBuilderAPI_Collect myHist;
};
#endif // _BRepBuilderAPI_HullTransform_HeaderFile
