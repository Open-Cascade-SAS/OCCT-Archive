// Copyright (c) 2017-2021 OPEN CASCADE SAS
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

#ifndef _Dynamics_CollisionBRepShapeConvex_HeaderFile
#define _Dynamics_CollisionBRepShapeConvex_HeaderFile

#include <Dynamics_CollisionBRepShape.hxx>

//! Define the collision body from triangulation data (convex).
class Dynamics_CollisionBRepShapeConvex : public Dynamics_CollisionBRepShape
{
  DEFINE_STANDARD_RTTIEXT(Dynamics_CollisionBRepShapeConvex, Dynamics_CollisionBRepShape)
public:

  //! Main constructor.
  Dynamics_CollisionBRepShapeConvex (const TopoDS_Shape& theShape,
                                     const gp_Trsf& theTrsf,
                                     Dynamics_CollisionBRepFlags theFlags = Dynamics_CollisionBRepFlags_CONVEX_DEFAULTS)
  : Dynamics_CollisionBRepShape (theShape, theFlags), myLocation (theTrsf) {}

  //! Return location.
  virtual const gp_Trsf& Location() const Standard_OVERRIDE { return myLocation; }

protected:
  gp_Trsf myLocation;
};

#endif // _Dynamics_CollisionBRepShapeConvex_HeaderFile
