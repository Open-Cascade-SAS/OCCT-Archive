// Copyright (c) 1995-1999 Matra Datavision
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


#include <gp_Cylinder.hxx>
#include <gp_Sphere.hxx>
#include <gp_Vec.hxx>
#include <IntSurf_Quadric.hxx>
#include <IntSurf_QuadricTool.hxx>

void IntSurf_QuadricTool::ValueAndGradient (const IntSurf_Quadric& Quad,
                                            const Standard_Real X,
                                            const Standard_Real Y,
                                            const Standard_Real Z,
                                            Standard_Real& Val,
                                            gp_Vec& Grad,
                                            gp_Vec& D1U,
                                            gp_Vec& D1V)
{
  gp_Pnt thePoint(X,Y,Z);
  Quad.ValAndGrad(thePoint, Val, Grad);
  Standard_Real U, V;
  Quad.Parameters(thePoint, U, V);
  Quad.D1(U, V, thePoint, D1U, D1V);
}

void IntSurf_QuadricTool::D2 (const IntSurf_Quadric& Quad,
                              const gp_Pnt& aPoint,
                              gp_Vec& D1U,
                              gp_Vec& D1V,
                              gp_Vec& D2U,
                              gp_Vec& D2V,
                              gp_Vec& D2UV)
{
  Standard_Real U, V;
  Quad.Parameters(aPoint, U, V);
  gp_Pnt thePoint;
  Quad.D1(U, V, thePoint, D1U, D1V);
  D2U = Quad.DN(U, V, 2, 0);
  D2V = Quad.DN(U, V, 0, 2);
  D2UV = Quad.DN(U, V, 1, 1);
}

Standard_Real IntSurf_QuadricTool::Tolerance (const IntSurf_Quadric& Q) {
  switch (Q.TypeQuadric()) {
  case GeomAbs_Sphere:
    return 2.e-6*Q.Sphere().Radius();
  case GeomAbs_Cylinder:
    return 2.e-6*Q.Cylinder().Radius();
  default: 
    break;
  }
  return(1e-6);
}

