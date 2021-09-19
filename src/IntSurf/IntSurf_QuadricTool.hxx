// Created on: 1992-09-30
// Created by: Jacques GOUSSARD
// Copyright (c) 1992-1999 Matra Datavision
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

#ifndef _IntSurf_QuadricTool_HeaderFile
#define _IntSurf_QuadricTool_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Standard_Real.hxx>
class IntSurf_Quadric;
class gp_Vec;
class gp_Pnt;


//! This class provides a tool on a quadric that can be
//! used to instantiates the Walking algorithms (see
//! package IntWalk) with a Quadric from IntSurf
//! as implicit surface.
class IntSurf_QuadricTool 
{
public:

  DEFINE_STANDARD_ALLOC

  
  //! Returns the value of the function.
    static Standard_Real Value (const IntSurf_Quadric& Quad,
                                const Standard_Real X,
                                const Standard_Real Y,
                                const Standard_Real Z);
  
  //! Returns the gradient of the function.
    static void Gradient (const IntSurf_Quadric& Quad,
                          const Standard_Real X,
                          const Standard_Real Y,
                          const Standard_Real Z,
                          gp_Vec& V);
  
  //! Returns the value and the gradient.
  Standard_EXPORT static   void ValueAndGradient (const IntSurf_Quadric& Quad,
                                                  const Standard_Real X,
                                                  const Standard_Real Y,
                                                  const Standard_Real Z,
                                                  Standard_Real& Val,
                                                  gp_Vec& Grad,
                                                  gp_Vec& D1U_ISurf,
                                                  gp_Vec& D1V_ISurf) ;
  
  //! Returns all first and second derivatives.
  Standard_EXPORT static   void D2 (const IntSurf_Quadric& Quad,
                                    const gp_Pnt& Point,
                                    gp_Vec& D1U,
                                    gp_Vec& D1V,
                                    gp_Vec& D2U,
                                    gp_Vec& D2V,
                                    gp_Vec& D2UV) ;
  
  //! returns the tolerance of the zero of the implicit function
  Standard_EXPORT static Standard_Real Tolerance (const IntSurf_Quadric& Quad);




protected:





private:





};


#include <IntSurf_QuadricTool.lxx>





#endif // _IntSurf_QuadricTool_HeaderFile
