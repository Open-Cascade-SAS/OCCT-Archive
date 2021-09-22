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

#include <Dynamics_CollisionBody.hxx>

#include <Dynamics_Internal.hxx>
#include <Dynamics_CollisionShape.hxx>
#include <Dynamics_Constraint.hxx>
#include <Dynamics_MotionState.hxx>
#include <Dynamics_World.hxx>

namespace
{
#ifdef HAVE_BULLET
  static btVector3 gpXyz2Vec3Bt (const gp_XYZ& theXYZ)
  {
    return btVector3 (btScalar(theXYZ.X()), btScalar(theXYZ.Y()), btScalar(theXYZ.Z()));
  }
#endif
}

IMPLEMENT_STANDARD_RTTIEXT(Dynamics_CollisionBody, Standard_Transient)

// =======================================================================
// function : Dynamics_CollisionBody
// purpose  :
// =======================================================================
Dynamics_CollisionBody::Dynamics_CollisionBody()
: myWorld (NULL)
{
  //
}

// =======================================================================
// function : Dynamics_CollisionBody
// purpose  :
// =======================================================================
Dynamics_CollisionBody::Dynamics_CollisionBody (const Handle(Dynamics_CollisionShape)& theShape)
: myWorld(NULL)
{
  SetShape (theShape);
}

// =======================================================================
// function : ~Dynamics_CollisionBody
// purpose  :
// =======================================================================
Dynamics_CollisionBody::~Dynamics_CollisionBody()
{
  clearMain();
}

// =======================================================================
// function : SetShape
// purpose  :
// =======================================================================
void Dynamics_CollisionBody::SetShape (const Handle(Dynamics_CollisionShape)& theShape,
                                          const gp_Trsf& theTrsf)
{
  myShape = theShape;
  myMotionState.Nullify();
#ifdef HAVE_BULLET

  // btCollisionObject cannot hold NULL collision shape (no NULL check in Bullet),
  // so remove object from the world, by keep reference for another ::SetShape() call
  bool toRemoveOld = myShape.IsNull(), toAdd = false;
  if (btRigidBody* aRigidBody = myBody.get() != NULL ? btRigidBody::upcast (myBody.get()) : NULL)
  {
    (void )aRigidBody;
    toRemoveOld = true;
  }
  if (toRemoveOld)
  {
    if (!myConstraint.IsNull())
    {
      if (myConstraint->DynamicsWorld() != nullptr)
      {
        myConstraint->DynamicsWorld()->RemoveConstraint (myConstraint);
      }
      myConstraint.Nullify();
    }

    Dynamics_World* aWorld = myWorld;
    removeFromWorld();
    myWorld = aWorld;
    myBody.reset();
  }
  if (myShape.IsNull())
  {
    return;
  }

  if (myBody.get() == NULL)
  {
    toAdd = true;
    myBody.reset (new btCollisionObject());
    myBody->setCollisionFlags (myBody->getCollisionFlags() | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);
  }

  btTransform aTrsfBt;
  Dynamics_MotionState::TrsfBtFromGp (aTrsfBt, theTrsf);

  myBody->setCollisionShape (myShape->CollisionShape());
  myBody->setWorldTransform (aTrsfBt);
  if (toAdd && myWorld != NULL)
  {
    addToWorld (myWorld);
  }
#else
  (void )theTrsf;
#endif
}

// =======================================================================
// function : SetShape
// purpose  :
// =======================================================================
void Dynamics_CollisionBody::SetShape (const Handle(Dynamics_CollisionShape)& theShape,
                                          const Handle(Dynamics_MotionState)& theMotionState,
                                          const float theMass)
{
  myShape = theShape;
  myMotionState = theMotionState;
#ifdef HAVE_BULLET
  bool toAdd = false, toRemoveOld = true;
  if (toRemoveOld)
  {
    if (!myConstraint.IsNull())
    {
      if (myConstraint->DynamicsWorld() != nullptr)
      {
        myConstraint->DynamicsWorld()->RemoveConstraint (myConstraint);
      }
      myConstraint.Nullify();
    }

    Dynamics_World* aWorld = myWorld;
    removeFromWorld();
    myWorld = aWorld;
    myBody.reset();
  }

  if (myBody.get() == NULL)
  {
    toAdd = true;
    btVector3 aLocalInertia (0.0f, 0.0f, 0.0f);
    myShape->CollisionShape()->calculateLocalInertia (theMass, aLocalInertia);

    btRigidBody::btRigidBodyConstructionInfo aBodyInfo (theMass, myMotionState->MotionState(), myShape->CollisionShape(), aLocalInertia);
    myBody.reset (new btRigidBody (aBodyInfo));


    myBody->setFriction (1.0f);
    myBody->setRollingFriction (0.1f);
    myBody->setSpinningFriction (0.1f);
    myBody->setAnisotropicFriction (myShape->CollisionShape()->getAnisotropicRollingFrictionDirection(),
                                    btCollisionObject::CF_ANISOTROPIC_ROLLING_FRICTION);
  }

  if (toAdd && myWorld != NULL)
  {
    addToWorld (myWorld);
  }
#else
  (void )theMass;
#endif
}

// =======================================================================
// function : addToWorld
// purpose  :
// =======================================================================
void Dynamics_CollisionBody::addToWorld (Dynamics_World* theWorld)
{
  removeFromWorld();
#ifdef HAVE_BULLET
  myWorld = theWorld;
  if (myBody.get() == NULL || myWorld == NULL || myShape.IsNull())
  {
    return;
  }

  btDiscreteDynamicsWorld* aWorld = myWorld->myDynamicsWorld.get();
  if (btRigidBody* aRigidBody = btRigidBody::upcast (myBody.get()))
  {
    ++myWorld->myNbRigidBodies;
    aWorld->addRigidBody (aRigidBody);
  }
  else
  {
    aWorld->addCollisionObject (myBody.get());
  }
  if (myAction.get() != NULL)
  {
    aWorld->addAction (myAction.get());
  }
#else
  (void )theWorld;
#endif
}

// =======================================================================
// function : removeFromWorld
// purpose  :
// =======================================================================
void Dynamics_CollisionBody::removeFromWorld()
{
#ifdef HAVE_BULLET
  if (myWorld != NULL && myBody.get() != NULL)
  {
    btDiscreteDynamicsWorld* aWorld = myWorld->myDynamicsWorld.get();
    if (myAction.get() != NULL)
    {
      aWorld->removeAction (myAction.get());
    }
    if (btRigidBody* aRigidBody = btRigidBody::upcast (myBody.get()))
    {
      aWorld->removeRigidBody (aRigidBody);
      --myWorld->myNbRigidBodies;
    }
    else
    {
      aWorld->removeCollisionObject (myBody.get());
    }
  }
  myWorld = NULL;
#endif
}

// =======================================================================
// function : clearMain
// purpose  :
// =======================================================================
void Dynamics_CollisionBody::clearMain()
{
  removeFromWorld();
#ifdef HAVE_BULLET
  myAction.reset();
  myBody.reset();
  myMotionState.Nullify();
#endif
}

// =======================================================================
// function : WorldPosition
// purpose  :
// =======================================================================
gp_Pnt Dynamics_CollisionBody::WorldPosition() const
{
#ifdef HAVE_BULLET
  if (myBody.get() == NULL)
  {
    return gp_Pnt (0.0, 0.0, 0.0);
  }

  const btTransform aTrsfBt = myBody->getWorldTransform();
  return gp_Pnt (aTrsfBt.getOrigin().x(), aTrsfBt.getOrigin().y(), aTrsfBt.getOrigin().z());
#else
  return gp_Pnt (0.0, 0.0, 0.0);
#endif
}

// =======================================================================
// function : WorldLocation
// purpose  :
// =======================================================================
gp_Trsf Dynamics_CollisionBody::WorldLocation() const
{
  gp_Trsf aTrsf;
#ifdef HAVE_BULLET
  if (myBody.get() == NULL)
  {
    return aTrsf;
  }

  const btTransform  aTrsfBt  = myBody->getWorldTransform();
  Dynamics_MotionState::TrsfGpFromBt (aTrsf, aTrsfBt);
#endif
  return aTrsf;
}

// =======================================================================
// function : SetWorldLocation
// purpose  :
// =======================================================================
void Dynamics_CollisionBody::SetWorldLocation (const gp_Trsf& theLoc)
{
#ifdef HAVE_BULLET
  if (myBody.get() != NULL)
  {
    btTransform aTrsfBt;
    Dynamics_MotionState::TrsfBtFromGp (aTrsfBt, theLoc);
    myBody->setWorldTransform (aTrsfBt);
  }
#else
  (void )theLoc;
#endif
}

// =======================================================================
// function : WorldMotionLocation
// purpose  :
// =======================================================================
gp_Trsf Dynamics_CollisionBody::WorldMotionLocation() const
{
  gp_Trsf aTrsf;
#ifdef HAVE_BULLET
  btRigidBody* aBody = btRigidBody::upcast (myBody.get());
  if (aBody == NULL || aBody->getMotionState() == NULL)
  {
    return WorldLocation();
  }

  btTransform aTrsfBt;
  aBody->getMotionState()->getWorldTransform (aTrsfBt);
  Dynamics_MotionState::TrsfGpFromBt (aTrsf, aTrsfBt);
#endif
  return aTrsf;
}

// =======================================================================
// function : ApplyCentralForce
// purpose  :
// =======================================================================
void Dynamics_CollisionBody::ApplyCentralForce (const gp_XYZ& theForce)
{
#ifdef HAVE_BULLET
  btRigidBody* aBody = btRigidBody::upcast (myBody.get());
  if (aBody == NULL || aBody->getMotionState() == NULL)
  {
    return;
  }

  aBody->activate (true);
  aBody->applyCentralForce (gpXyz2Vec3Bt (theForce));
#else
  (void )theForce;
#endif
}

// =======================================================================
// function : ApplyCentralImpulse
// purpose  :
// =======================================================================
void Dynamics_CollisionBody::ApplyCentralImpulse (const gp_XYZ& theImpulse)
{
#ifdef HAVE_BULLET
  btRigidBody* aBody = btRigidBody::upcast (myBody.get());
  if (aBody == NULL || aBody->getMotionState() == NULL)
  {
    return;
  }

  aBody->activate (true);
  aBody->applyCentralImpulse (gpXyz2Vec3Bt (theImpulse));
#else
  (void )theImpulse;
#endif
}

// =======================================================================
// function : ApplyTorque
// purpose  :
// =======================================================================
void Dynamics_CollisionBody::ApplyTorque (const gp_XYZ& theTorque)
{
#ifdef HAVE_BULLET
  btRigidBody* aBody = btRigidBody::upcast (myBody.get());
  if (aBody == NULL || aBody->getMotionState() == NULL)
  {
    return;
  }

  aBody->activate (true);
  aBody->applyTorque (gpXyz2Vec3Bt (theTorque));
#else
  (void )theTorque;
#endif
}

// =======================================================================
// function : ApplyForce
// purpose  :
// =======================================================================
void Dynamics_CollisionBody::ApplyForce (const gp_XYZ& theForce,
                                            const gp_Pnt& theRelPos)
{
#ifdef HAVE_BULLET
  btRigidBody* aBody = btRigidBody::upcast (myBody.get());
  if (aBody == NULL || aBody->getMotionState() == NULL)
  {
    return;
  }

  aBody->activate (true);
  aBody->applyForce (gpXyz2Vec3Bt (theForce), gpXyz2Vec3Bt (theRelPos.XYZ()));
#else
  (void )theForce;
  (void )theRelPos;
#endif
}

// =======================================================================
// function : ApplyImpulse
// purpose  :
// =======================================================================
void Dynamics_CollisionBody::ApplyImpulse (const gp_XYZ& theImpulse,
                                              const gp_Pnt& theRelPos)
{
#ifdef HAVE_BULLET
  btRigidBody* aBody = btRigidBody::upcast (myBody.get());
  if (aBody == NULL || aBody->getMotionState() == NULL)
  {
    return;
  }

  aBody->activate (true);
  aBody->applyImpulse (gpXyz2Vec3Bt (theImpulse), gpXyz2Vec3Bt (theRelPos.XYZ()));
#else
  (void )theImpulse;
  (void )theRelPos;
#endif
}

// =======================================================================
// function : ApplyTorqueImpulse
// purpose  :
// =======================================================================
void Dynamics_CollisionBody::ApplyTorqueImpulse (const gp_XYZ& theTorque)
{
#ifdef HAVE_BULLET
  btRigidBody* aBody = btRigidBody::upcast (myBody.get());
  if (aBody == NULL || aBody->getMotionState() == NULL)
  {
    return;
  }

  aBody->activate (true);
  aBody->applyTorqueImpulse (gpXyz2Vec3Bt (theTorque));
#else
  (void )theTorque;
#endif
}
