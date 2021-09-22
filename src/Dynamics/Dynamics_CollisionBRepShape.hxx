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

#ifndef _Dynamics_CollisionBRepShape_HeaderFile
#define _Dynamics_CollisionBRepShape_HeaderFile

#include <Dynamics_CollisionShape.hxx>
#include <Dynamics_CollisionBRepFlags.hxx>

class TopoDS_Shape;

//! Define the collision body from triangulation data.
class Dynamics_CollisionBRepShape : public Dynamics_CollisionShape
{
  DEFINE_STANDARD_RTTIEXT(Dynamics_CollisionBRepShape, Dynamics_CollisionShape)
public:

  //! Constructor.
  Dynamics_CollisionBRepShape (const TopoDS_Shape& theShape,
                               Dynamics_CollisionBRepFlags theFlags = Dynamics_CollisionBRepFlags_DEFAULTS)
  {
    Init (theShape, theFlags);
  }

  //! Initialize from triangulation.
  Standard_EXPORT bool Init (const TopoDS_Shape& theShape,
                             Dynamics_CollisionBRepFlags theFlags);

};

#endif // _Dynamics_CollisionBRepShape_HeaderFile
