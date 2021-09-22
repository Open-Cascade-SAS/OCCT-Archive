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

#include <Dynamics_CollisionShape.hxx>

#include <Dynamics_Internal.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Dynamics_CollisionShape, Standard_Transient)

// =======================================================================
// function : Dynamics_CollisionShape
// purpose  :
// =======================================================================
Dynamics_CollisionShape::Dynamics_CollisionShape()
: myShape (NULL)
{
  //
}

// =======================================================================
// function : Dynamics_CollisionShape
// purpose  :
// =======================================================================
Dynamics_CollisionShape::Dynamics_CollisionShape (btCollisionShape* theShape)
: myShape (theShape)
{
  //
}

// =======================================================================
// function : ~Dynamics_CollisionShape
// purpose  :
// =======================================================================
Dynamics_CollisionShape::~Dynamics_CollisionShape()
{
  releaseShape();
}

// =======================================================================
// function : releaseShape
// purpose  :
// =======================================================================
void Dynamics_CollisionShape::releaseShape()
{
#ifdef HAVE_BULLET
  delete myShape;
#endif
  myShape = NULL;
}
