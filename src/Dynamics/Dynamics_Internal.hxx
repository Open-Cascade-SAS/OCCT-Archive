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

#ifndef _Dynamics_Internal_HeaderFile
#define _Dynamics_Internal_HeaderFile

#ifdef HAVE_BULLET
  #include <Standard_WarningsDisable.hxx>

  #include <btBulletDynamicsCommon.h>
  //#include <BulletCollision/Gimpact/btGImpactShape.h>
  #include <BulletCollision/CollisionDispatch/btGhostObject.h>
  #include <BulletCollision/CollisionShapes/btShapeHull.h>
  #include <BulletDynamics/Character/btKinematicCharacterController.h>

  #include <Standard_WarningsRestore.hxx>
#else
class btTransform {};
#endif

#endif // _Dynamics_Internal_HeaderFile
