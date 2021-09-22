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

#include <Dynamics_World.hxx>

#include <Dynamics_Internal.hxx>
#include <Dynamics_CollisionPlane.hxx>
#include <Dynamics_Constraint.hxx>
#include <Dynamics_Parameters.hxx>

#include <gp_Pln.hxx>

#ifdef HAVE_BULLET
  #include <Dynamics_DebugDrawer.hxx>

  #ifdef _MSC_VER
  #ifdef NDEBUG
    #pragma comment(lib, "BulletDynamics.lib")
    #pragma comment(lib, "BulletCollision.lib")
    #pragma comment(lib, "LinearMath.lib")
  #else
    #pragma comment(lib, "BulletDynamics_Debug.lib")
    #pragma comment(lib, "BulletCollision_Debug.lib")
    #pragma comment(lib, "LinearMath_Debug.lib")
  #endif
  #endif
#endif

IMPLEMENT_STANDARD_RTTIEXT(Dynamics_World, Standard_Transient)

// =======================================================================
// function : Dynamics_World
// purpose  :
// =======================================================================
Dynamics_World::Dynamics_World (const Handle(Dynamics_Parameters)& theDefaultStyle) :
#ifdef HAVE_BULLET
  myGhostPairCallback (new btGhostPairCallback()),
  myCollisionConfiguration (new btDefaultCollisionConfiguration()),
  myOverlappingPairCache (new btDbvtBroadphase()),
  mySolver (new btSequentialImpulseConstraintSolver()),
  myCharacter (new Dynamics_Character()),
  myGround (new Dynamics_CollisionBody()),
#endif
  myLengthUnitScale (1.0),
  myLastWorldTick (0.0),
  myFixedTimeStep (1.0 / 120.0),
  myNbMaxSubTicks (20),
  myNbRigidBodies (0)
{
  myDefaultDrawer = theDefaultStyle;
  if (theDefaultStyle.IsNull())
  {
    myDefaultDrawer = new Dynamics_Parameters();
  }
#ifdef HAVE_BULLET
  myCharacter->Init (theDefaultStyle->FirstPersonHeight(), theDefaultStyle->FirstPersonWidth(), theDefaultStyle->FirstPersonStepHeight());
  myOverlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback (myGhostPairCallback.get());
  myDispatcher.reset (new btCollisionDispatcher (myCollisionConfiguration.get()));
  myDynamicsWorld.reset (new btDiscreteDynamicsWorld (myDispatcher.get(), myOverlappingPairCache.get(), mySolver.get(), myCollisionConfiguration.get()));
  myDynamicsWorld->setGravity (btVector3 (0.0f, 0.0f, btScalar(-9.8 * myLengthUnitScale)));

  myGround->addToWorld (this);
#endif
}

// =======================================================================
// function : ~Dynamics_World
// purpose  :
// =======================================================================
Dynamics_World::~Dynamics_World()
{
  for (NCollection_IndexedMap<Handle(Dynamics_Constraint)>::Iterator aConstrIter (myConstraints); aConstrIter.More(); aConstrIter.Next())
  {
    aConstrIter.Value()->removeFromWorld();
  }
  for (NCollection_IndexedMap<Handle(Dynamics_CollisionBody)>::Iterator aBodyIter (myCollisionBodies); aBodyIter.More(); aBodyIter.Next())
  {
    aBodyIter.Value()->removeFromWorld();
  }
  if (!myCharacter.IsNull())
  {
    myCharacter->removeFromWorld();
  }
  if (!myGround.IsNull())
  {
    myGround->removeFromWorld();
  }
  myConstraints.Clear();
  myCollisionBodies.Clear();
  myCharacter.Nullify();

#ifdef HAVE_BULLET
  myDynamicsWorld.reset();
  mySolver.reset();
  myOverlappingPairCache.reset();
  myDispatcher.reset();
  myCollisionConfiguration.reset();
  myGhostPairCallback.reset();
#endif
}

// =======================================================================
// function : UpdateFirstPersonSize
// purpose  :
// =======================================================================
void Dynamics_World::UpdateFirstPersonSize()
{
  if (!myCharacter.IsNull())
  {
    myCharacter->Init (myLengthUnitScale * myDefaultDrawer->FirstPersonHeight(), myLengthUnitScale * myDefaultDrawer->FirstPersonWidth(), myLengthUnitScale * myDefaultDrawer->FirstPersonStepHeight());
  }
}

// =======================================================================
// function : SetLengthUnitScale
// purpose  :
// =======================================================================
void Dynamics_World::SetLengthUnitScale (double theScale)
{
  myLengthUnitScale = theScale;
#ifdef HAVE_BULLET
  myDynamicsWorld->setGravity (btVector3 (0.0f, 0.0f, btScalar(-9.8 * myLengthUnitScale)));
#endif
  UpdateFirstPersonSize();
}

// =======================================================================
// function : SetSceneBoundaries
// purpose  :
// =======================================================================
void Dynamics_World::SetSceneBoundaries (const Bnd_Box& theBndBox)
{
  mySceneBox = theBndBox;
  if (mySceneBox.IsVoid())
  {
    return;
  }

#ifdef HAVE_BULLET
  const gp_XYZ aMin = mySceneBox.CornerMin().XYZ();
  const gp_XYZ aMax = mySceneBox.CornerMax().XYZ();
  gp_XYZ aCenter = (aMin + aMax) * 0.5;
  aCenter.SetZ (aMin.Z());

  gp_Pln aPlane (gp_Pnt (0.0, 0.0, 0.0), gp_Dir (0.0, 0.0, 1.0));
  Handle(Dynamics_CollisionPlane) aPlaneShape = new Dynamics_CollisionPlane (aPlane);

  gp_Trsf aTrsf;
  aTrsf.SetTranslation (gp_Vec (aCenter));
  myGround->SetShape (aPlaneShape, aTrsf);
#endif
}

// =======================================================================
// function : AddCollisionBody
// purpose  :
// =======================================================================
void Dynamics_World::AddCollisionBody (const Handle(Dynamics_CollisionBody)& theBody)
{
  const int anExtendOld = myCollisionBodies.Extent();
  if (myCollisionBodies.Add (theBody) > anExtendOld)
  {
  #ifdef HAVE_BULLET
    theBody->addToWorld (this);
  #endif
  }
}

// =======================================================================
// function : RemoveCollisionBody
// purpose  :
// =======================================================================
void Dynamics_World::RemoveCollisionBody (const Handle(Dynamics_CollisionBody)& theBody)
{
  Handle(Dynamics_CollisionBody) aTmpCopy = theBody;
  if (myCollisionBodies.RemoveKey (theBody))
  {
  #ifdef HAVE_BULLET
    theBody->removeFromWorld();
  #endif
  }
}

// =======================================================================
// function : AddConstraint
// purpose  :
// =======================================================================
void Dynamics_World::AddConstraint (const Handle(Dynamics_Constraint)& theConstraint)
{
  const int anExtendOld = myConstraints.Extent();
  if (myConstraints.Add (theConstraint) > anExtendOld)
  {
  #ifdef HAVE_BULLET
    theConstraint->addToWorld (this);
  #endif
  }
}

// =======================================================================
// function : RemoveConstraint
// purpose  :
// =======================================================================
void Dynamics_World::RemoveConstraint (const Handle(Dynamics_Constraint)& theConstraint)
{
  Handle(Dynamics_Constraint) aTmpCopy = theConstraint;
  if (myConstraints.RemoveKey (theConstraint))
  {
  #ifdef HAVE_BULLET
    theConstraint->removeFromWorld();
  #endif
  }
}

// =======================================================================
// function : SetPaused
// purpose  :
// =======================================================================
void Dynamics_World::SetPaused (bool theToPause)
{
  if (IsPaused() == theToPause)
  {
    return;
  }

  if (theToPause)
  {
    myWorldTimer.Stop();
  }
  else
  {
    myWorldTimer.Start();
  }
}

// =======================================================================
// function : SetEnableDebugDrawer
// purpose  :
// =======================================================================
bool Dynamics_World::SetEnableDebugDrawer (bool theToDebug,
                                           AIS_InteractiveContext* theCtx)
{
#ifdef HAVE_BULLET
  if (theToDebug)
  {
    if (myDebugDrawer.get() == nullptr)
    {
      myDebugDrawer.reset (new Dynamics_DebugDrawer());
      myDynamicsWorld->setDebugDrawer (myDebugDrawer.get());
      return true;
    }
  }
  else if (myDebugDrawer.get() != nullptr)
  {
    const bool toUpdate = !myDebugDrawer->IsEmpty();
    myDynamicsWorld->setDebugDrawer (nullptr);
    myDebugDrawer->FrameClear (theCtx);
    myDebugDrawer.reset();
    return toUpdate;
  }
#else
  (void )theToDebug;
  (void )theCtx;
#endif
  return false;
}

// =======================================================================
// function : StepSimulation
// purpose  :
// =======================================================================
bool Dynamics_World::StepSimulation (const Handle(AIS_InteractiveContext)& theCtx)
{
  const double aTime     = myWorldTimer.ElapsedTime();
  const double aTimeDiff = aTime - myLastWorldTick;
  SetPaused (false);
  if (Abs (aTimeDiff - myLastWorldTick) < gp::Resolution())
  {
    return false;
  }

  myLastWorldTick = aTime;
#ifdef HAVE_BULLET
  if (myDynamicsWorld->stepSimulation (btScalar(aTimeDiff), myNbMaxSubTicks, btScalar(myFixedTimeStep)) <= 0)
  {
    return false;
  }

  if (Dynamics_DebugDrawer* aDebugDraw = myDebugDrawer.get())
  {
    if (!theCtx.IsNull())
    {
      aDebugDraw->FrameBegin (theCtx);
      myDynamicsWorld->debugDrawWorld();
      aDebugDraw->FrameEnd (theCtx);
    }
  }
  return true;
#else
  (void )theCtx;
  return false;
#endif
}
