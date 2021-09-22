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

#include <Dynamics_Constraint.hxx>

#include <Dynamics_Internal.hxx>
#include <Dynamics_CollisionBody.hxx>
#include <Dynamics_World.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Dynamics_Constraint, Standard_Transient)

// =======================================================================
// function : Dynamics_Constraint
// purpose  :
// =======================================================================
Dynamics_Constraint::Dynamics_Constraint()
: myConstraint (nullptr)
{
  //
}

// =======================================================================
// function : ~Dynamics_Constraint
// purpose  :
// =======================================================================
Dynamics_Constraint::~Dynamics_Constraint()
{
  releaseConstraint();
}

// =======================================================================
// function : releaseConstraint
// purpose  :
// =======================================================================
void Dynamics_Constraint::releaseConstraint()
{
  removeFromWorld();
#ifdef HAVE_BULLET
  delete myConstraint;
#endif
  myConstraint = nullptr;
}

// =======================================================================
// function : addToWorld
// purpose  :
// =======================================================================
void Dynamics_Constraint::addToWorld (Dynamics_World* theWorld)
{
  removeFromWorld();
#ifdef HAVE_BULLET
  myWorld = theWorld;
  if (myConstraint == nullptr || theWorld == nullptr)
  {
    return;
  }

  btDiscreteDynamicsWorld* aWorld = theWorld->myDynamicsWorld.get();
  aWorld->addConstraint (myConstraint);
#else
  (void )theWorld;
#endif
}

// =======================================================================
// function : removeFromWorld
// purpose  :
// =======================================================================
void Dynamics_Constraint::removeFromWorld()
{
#ifdef HAVE_BULLET
  if (myWorld != nullptr && myConstraint != nullptr)
  {
    btDiscreteDynamicsWorld* aWorld = myWorld->myDynamicsWorld.get();
    aWorld->removeConstraint (myConstraint);
  }
  myWorld = nullptr;
#endif
}

// =======================================================================
// function : CreateGeneric6Dof
// purpose  :
// =======================================================================
void Dynamics_Constraint::CreateGeneric6Dof (const Handle(Dynamics_CollisionBody)& theBody,
                                             const gp_XYZ& theLinearLower,
                                             const gp_XYZ& theLinearUpper,
                                             const gp_XYZ& theAngularLower,
                                             const gp_XYZ& theAngularUpper)
{
  Dynamics_World* aWorld = myWorld;
  releaseConstraint();
  if (theBody.IsNull())
  {
    throw Standard_ProgramError ("Dynamics_Constraint, Internal Error - NULL Rigid Body");
  }

#ifdef HAVE_BULLET
  btRigidBody* aRigidBody = btRigidBody::upcast (theBody->collisionBody());
  if (aRigidBody == nullptr)
  {
    throw Standard_ProgramError ("Dynamics_Constraint, Internal Error - no Rigid Body");
  }

  const btTransform aFrameInBody = btTransform::getIdentity();
  myConstraint = new btGeneric6DofConstraint (*aRigidBody, aFrameInBody, false);
  myConstraint->setDbgDrawSize (5.0f);
  SetLinearLimits (theLinearLower,  theLinearUpper);
  SetAngularLimits(theAngularLower, theAngularUpper);
  theBody->myConstraint = this;
  if (aWorld != nullptr)
  {
    addToWorld (aWorld);
  }
  else if (theBody->myWorld != nullptr)
  {
    theBody->myWorld->AddConstraint (this);
    //addToWorld (theBody->myWorld);
  }
#else
  (void )aWorld;
  (void )theLinearLower;
  (void )theLinearUpper;
  (void )theAngularLower;
  (void )theAngularUpper;
#endif
}

// =======================================================================
// function : LinearLimits
// purpose  :
// =======================================================================
bool Dynamics_Constraint::LinearLimits (gp_XYZ& theLower, gp_XYZ& theUpper) const
{
#ifdef HAVE_BULLET
  if (const btGeneric6DofConstraint* aConstr = dynamic_cast<const btGeneric6DofConstraint*>(myConstraint))
  {
    btVector3 aLower, anUpper;
    aConstr->getLinearLowerLimit (aLower);
    aConstr->getLinearUpperLimit (anUpper);
    theLower.SetCoord (aLower.x(),  aLower.y(),  aLower.z());
    theUpper.SetCoord (anUpper.x(), anUpper.y(), anUpper.z());
    return true;
  }
#endif
  theLower.SetCoord ( 1.0,  1.0,  1.0);
  theUpper.SetCoord (-1.0, -1.0, -1.0);
  return false;
}

// =======================================================================
// function : SetLinearLimits
// purpose  :
// =======================================================================
void Dynamics_Constraint::SetLinearLimits (const gp_XYZ& theLower, const gp_XYZ& theUpper)
{
#ifdef HAVE_BULLET
  if (btGeneric6DofConstraint* aConstr = dynamic_cast<btGeneric6DofConstraint*>(myConstraint))
  {
    aConstr->setLinearLowerLimit (btVector3 ((btScalar )theLower.X(),  (btScalar )theLower.Y(),  (btScalar )theLower.Z()));
    aConstr->setLinearUpperLimit (btVector3 ((btScalar )theUpper.X(),  (btScalar )theUpper.Y(),  (btScalar )theUpper.Z()));
    return;
  }
#else
  (void )theLower;
  (void )theUpper;
#endif
  throw Standard_ProgramError ("Dynamics_Constraint::SetLinearLimits(), invalid constraint type");
}

// =======================================================================
// function : AngularLimits
// purpose  :
// =======================================================================
bool Dynamics_Constraint::AngularLimits (gp_XYZ& theLower, gp_XYZ& theUpper) const
{
#ifdef HAVE_BULLET
  if (const btGeneric6DofConstraint* aConstr = dynamic_cast<const btGeneric6DofConstraint*>(myConstraint))
  {
    btVector3 aLower, anUpper;
    aConstr->getAngularLowerLimit (aLower);
    aConstr->getAngularUpperLimit (anUpper);
    theLower.SetCoord (aLower.x(),  aLower.y(),  aLower.z());
    theUpper.SetCoord (anUpper.x(), anUpper.y(), anUpper.z());
    return true;
  }
#endif
  theLower.SetCoord ( 1.0,  1.0,  1.0);
  theUpper.SetCoord (-1.0, -1.0, -1.0);
  return false;
}

// =======================================================================
// function : SetAngularLimits
// purpose  :
// =======================================================================
void Dynamics_Constraint::SetAngularLimits (const gp_XYZ& theLower, const gp_XYZ& theUpper)
{
#ifdef HAVE_BULLET
  if (btGeneric6DofConstraint* aConstr = dynamic_cast<btGeneric6DofConstraint*>(myConstraint))
  {
    aConstr->setAngularLowerLimit (btVector3 ((btScalar )theLower.X(),  (btScalar )theLower.Y(),  (btScalar )theLower.Z()));
    aConstr->setAngularUpperLimit (btVector3 ((btScalar )theUpper.X(),  (btScalar )theUpper.Y(),  (btScalar )theUpper.Z()));
    return;
  }
#else
  (void )theLower;
  (void )theUpper;
#endif
  throw Standard_ProgramError ("Dynamics_Constraint::SetAngularLimits(), invalid constraint type");
}
