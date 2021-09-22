// Copyright (c) 2018-2021 OPEN CASCADE SAS
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

#ifndef _Dynamics_CollisionBoxShape_HeaderFile
#define _Dynamics_CollisionBoxShape_HeaderFile

#include <Dynamics_CollisionShape.hxx>

#include <Bnd_OBB.hxx>

//! Define the collision shape as OBB.
class Dynamics_CollisionBoxShape : public Dynamics_CollisionShape
{
  DEFINE_STANDARD_RTTIEXT(Dynamics_CollisionBoxShape, Dynamics_CollisionShape)
public:

  //! Constructor from OBB.
  Standard_EXPORT Dynamics_CollisionBoxShape (const Bnd_OBB& theOBB);

  //! Constructor from AABB.
  Standard_EXPORT Dynamics_CollisionBoxShape (const Bnd_Box& theAABB);

  //! Return shape location.
  virtual const gp_Trsf& Location() const Standard_OVERRIDE { return myLocation; }

protected:
  gp_Trsf myLocation;
};

#endif // _Dynamics_CollisionBoxShape_HeaderFile
