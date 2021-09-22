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

#include <Dynamics_CollisionCompoundShape.hxx>

#include <Dynamics_Internal.hxx>
#include <Dynamics_MotionState.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Dynamics_CollisionCompoundShape, Dynamics_CollisionShape)

// =======================================================================
// function : Dynamics_CollisionCompoundShape
// purpose  :
// =======================================================================
Dynamics_CollisionCompoundShape::Dynamics_CollisionCompoundShape()
{
  //
}

// =======================================================================
// function : ~Dynamics_CollisionCompoundShape
// purpose  :
// =======================================================================
Dynamics_CollisionCompoundShape::~Dynamics_CollisionCompoundShape()
{
  //
}

// =======================================================================
// function : Init
// purpose  :
// =======================================================================
bool Dynamics_CollisionCompoundShape::Init()
{
  releaseShape();
  if (mySubShapes.IsEmpty())
  {
    return false;
  }

#ifdef HAVE_BULLET
  btTransform aTrsfBt;
  aTrsfBt.setIdentity();

  btCompoundShape* aCompShape = new btCompoundShape();
  myShape = aCompShape;
  for (int aChildIter = mySubShapes.Lower(); aChildIter <= mySubShapes.Upper(); ++aChildIter)
  {
    const Handle(Dynamics_CollisionShape)& aShape = mySubShapes.Value (aChildIter);
    if (!aShape.IsNull()
      && aShape->IsValid())
    {
      const gp_Trsf& aTrsf = aShape->Location();
      Dynamics_MotionState::TrsfBtFromGp (aTrsfBt, aTrsf);
      aCompShape->addChildShape (aTrsfBt, aShape->myShape);
    }
  }
  if (aCompShape->getNumChildShapes() == 0)
  {
    releaseShape();
    return false;
  }
  return true;
#else
  return false;
#endif
}
