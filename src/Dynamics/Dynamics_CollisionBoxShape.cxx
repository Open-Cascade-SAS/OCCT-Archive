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

#include <Dynamics_CollisionBoxShape.hxx>

#include <Dynamics_Internal.hxx>

#include <gp_Ax3.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Dynamics_CollisionBoxShape, Dynamics_CollisionShape)

// =======================================================================
// function : Dynamics_CollisionBoxShape
// purpose  :
// =======================================================================
Dynamics_CollisionBoxShape::Dynamics_CollisionBoxShape (const Bnd_OBB& theOBB)
{
  if (!theOBB.IsVoid())
  {
  #ifdef HAVE_BULLET
    myShape = new btBoxShape (btVector3 (btScalar(theOBB.XHSize()), btScalar(theOBB.YHSize()), btScalar(theOBB.ZHSize())));
    myLocation.SetTransformation (gp_Ax3 (theOBB.Center(), theOBB.ZDirection(), theOBB.XDirection()), gp::XOY());
  #endif
  }
}

// =======================================================================
// function : Dynamics_CollisionBoxShape
// purpose  :
// =======================================================================
Dynamics_CollisionBoxShape::Dynamics_CollisionBoxShape (const Bnd_Box& theAABB)
{
  if (!theAABB.IsVoid())
  {
  #ifdef HAVE_BULLET
    const gp_XYZ aSize = theAABB.CornerMax().XYZ() - theAABB.CornerMin().XYZ();
    myShape = new btBoxShape (btVector3 (btScalar(aSize.X() * 0.5), btScalar(aSize.Y() * 0.5), btScalar(aSize.Z() * 0.5)));
    myLocation.SetTranslation ((theAABB.CornerMin().XYZ() + theAABB.CornerMax().XYZ()) * 0.5);
  #endif
  }
}
