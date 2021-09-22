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

#include <Dynamics_CollisionPlane.hxx>

#include <Dynamics_Internal.hxx>

#include <gp_Pln.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Dynamics_CollisionPlane, Dynamics_CollisionShape)

// =======================================================================
// function : InitPlane
// purpose  :
// =======================================================================
void Dynamics_CollisionPlane::InitPlane (const gp_Pln& thePlane)
{
  releaseShape();
#ifdef HAVE_BULLET
  // ABCD does not work as expected for unknown reason
  //NCollection_Vec4<double> aCoef;
  //thePlane.Coefficients (aCoef.x(), aCoef.y(), aCoef.z(), aCoef.w());
  //myShape = new btStaticPlaneShape (btVector3 (btScalar(aCoef.x()), btScalar(aCoef.y()), btScalar(aCoef.z())), btScalar(aCoef.w()));
  myShape = new btStaticPlaneShape (btVector3 (btScalar(thePlane.Axis().Direction().X()),
                                               btScalar(thePlane.Axis().Direction().Y()),
                                               btScalar(thePlane.Axis().Direction().Z())),
                                    btScalar(0.0));
#else
  (void )thePlane;
#endif
}
