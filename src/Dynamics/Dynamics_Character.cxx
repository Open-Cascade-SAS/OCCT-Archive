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

#include <Dynamics_Character.hxx>

#include <Dynamics_CollisionShape.hxx>
#include <Dynamics_Internal.hxx>
#include <Dynamics_World.hxx>

#include <Bnd_Box.hxx>
#include <gp_Vec.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Dynamics_Character, Dynamics_CollisionBody)

namespace
{
  static const double   THE_GRAVITY_G = 9.8;

#ifdef HAVE_BULLET
  static const btScalar THE_TIME_PRECISION = 0.001f;
  static const double   THE_DIST_PRECISION = 0.0001;

  //! Character controller.
  ATTRIBUTE_ALIGNED16(class) Dynamics_CharacterController : public btKinematicCharacterController
  {
  public:

    enum StableState
    {
      StableState_Reset,
      StableState_Stable,
      StableState_Stuck,
    };

  public:

    //! Main constructor.
    Dynamics_CharacterController (btPairCachingGhostObject* theGhostObject,
                                     btConvexShape* theConvexShape,
                                     btScalar theStepHeight,
                                     const btVector3& theUp)
    : btKinematicCharacterController (theGhostObject, theConvexShape, theStepHeight, theUp),
      myOldDirection (0.0f, 0.0f, 0.0f),
      myStableState (StableState_Reset)
    {
      //
    }

    //! Setup distance tolerance.
    void setDistancePrecision (btScalar theDistancePrec)
    {
      myDistancePrec = theDistancePrec;
    }

    //! Action interface.
    virtual void updateAction (btCollisionWorld* theCollisionWorld,
                               btScalar theDeltaTimeStep) Standard_OVERRIDE
    {
      const bool isUpdateNeeded = !m_useWalkDirection && m_velocityTimeInterval > THE_TIME_PRECISION && m_walkDirection.length() > myDistancePrec;
      if (myStableState != StableState_Reset)
      {
        return;
      }

      const btVector3 aLocBefore = m_ghostObject->getWorldTransform().getOrigin();
      btKinematicCharacterController::updateAction (theCollisionWorld, theDeltaTimeStep);

      const btVector3 aLocAfter = m_ghostObject->getWorldTransform().getOrigin();
      const btVector3 aDelta    = aLocAfter - aLocBefore;
      if (theDeltaTimeStep > THE_TIME_PRECISION)
      {
        if (aDelta.length() < myDistancePrec)
        {
          myStableState = isUpdateNeeded ? StableState_Stuck : StableState_Stable;
        }
      }
    }

    //! Setup walking.
    virtual void setVelocityForTimeInterval (const btVector3& theVelocity, btScalar theTimeInterval) Standard_OVERRIDE
    {
      if (theVelocity.length() > myDistancePrec)
      {
        const btVector3 aNewDirection = theVelocity.normalized();
        const btScalar  aDelta = (myOldDirection - aNewDirection).length();
        myOldDirection = aNewDirection;
        if (aDelta > myDistancePrec
         || myStableState == StableState_Stable)
        {
          myStableState = StableState_Reset;
        }
      }

      btKinematicCharacterController::setVelocityForTimeInterval (btVector3 (0.0f, 0.0f, 0.0f), 0.0f);
      btKinematicCharacterController::setVelocityForTimeInterval (theVelocity, theTimeInterval);
    }

    //! Reset stable state.
    void ResetStable()
    {
      myStableState = StableState_Reset;
      btKinematicCharacterController::setVelocityForTimeInterval (btVector3 (0.0f, 0.0f, 0.0f), 0.0f);
    }

  private:

    btVector3   myOldDirection;
    btScalar    myDistancePrec;
    StableState myStableState;

  };
#endif
}

// =======================================================================
// function : Dynamics_Character
// purpose  :
// =======================================================================
Dynamics_Character::Dynamics_Character (double theHumanHeight)
: myDefFallSpeed(0.0),
  myFallSpeed  (0.0),
  myHumanHeight(1.0),
  myHumanWidth (1.0),
  myStepHeight (1.0),
  myRunRatio   (3.0),
  myCrouchRatio(0.5),
  myCapsHalfHeight(1.0),
  myCapsMaxPenetr (0.1), // 0.2 is default
  myState (Dynamics_CharacterState_Stand)
{
  Init (theHumanHeight);
}

// =======================================================================
// function : ~Dynamics_Character
// purpose  :
// =======================================================================
Dynamics_Character::~Dynamics_Character()
{
  //
}

// =======================================================================
// function : Init
// purpose  :
// =======================================================================
void Dynamics_Character::Init (double theHumanHeight,
                                  double theHumanWidth,
                                  double theStepHeight)
{
  myHumanHeight = theHumanHeight;
  myHumanWidth  = theHumanWidth;
  myStepHeight  = theStepHeight;
  init();
}

// =======================================================================
// function : init
// purpose  :
// =======================================================================
void Dynamics_Character::init()
{
  Dynamics_World* aWorld = myWorld;
  const double aLenScale = aWorld != nullptr ? aWorld->LengthUnitScale() : 1.0;

  myVelocity.SetCoord (0.0, 0.0, 0.0);
  myGravity = myDefGravity = gp_Vec (0.0, 0.0, -THE_GRAVITY_G * 3.0 * aLenScale); // 3G acceleration
  myFallSpeed = myDefFallSpeed = 55.0 * aLenScale;
#ifdef HAVE_BULLET
  double aHumanHeight = myHumanHeight;
  double aHumanWidth  = myHumanWidth;
  double aStepHeight  = myStepHeight;
  if (myState == Dynamics_CharacterState_Crouch)
  {
    aHumanHeight = myHumanHeight * 0.5;
    aStepHeight  = myStepHeight  * 0.5;
  }
  myCapsHalfHeight = aHumanHeight * 0.5;

  btTransform aWorldLoc;
  aWorldLoc.setIdentity();
  if (myBody.get() != NULL)
  {
    aWorldLoc = myBody->getWorldTransform();
  }

  myCapsMaxPenetr = 0.1 * aLenScale;
  if (aWorld != NULL)
  {
    removeFromWorld();
  }

  if (!myCustomShape.IsNull())
  {
    const btConvexShape* aConvexShape = dynamic_cast<btConvexShape*> (myCustomShape->CollisionShape());
    if (aConvexShape == NULL)
    {
      throw Standard_ProgramError ("Dynamics_Character::init() - non-convex shape is set");
    }
    myShape = myCustomShape;
  }
  else
  {
    const double aCapsuleRadius = aHumanWidth * 0.5;
    const double aCapsuleHeight = aHumanHeight - 2.0 * aCapsuleRadius;

    btCapsuleShapeZ* aCapsule = new btCapsuleShapeZ (btScalar(aCapsuleRadius), btScalar(aCapsuleHeight));
    myShape = new Dynamics_CollisionShape (aCapsule);
  }

  btPairCachingGhostObject* aGhostObject = new btPairCachingGhostObject();
  aGhostObject->setWorldTransform (aWorldLoc);
  aGhostObject->setCollisionShape (myShape->CollisionShape());
  aGhostObject->setCollisionFlags (btCollisionObject::CF_CHARACTER_OBJECT);
  myBody.reset (aGhostObject);
  btConvexShape* aConvexShape = dynamic_cast<btConvexShape*> (myShape->CollisionShape());
  Dynamics_CharacterController* aCharacter = new Dynamics_CharacterController (aGhostObject, aConvexShape, btScalar (aStepHeight), btVector3 (0.0f, 0.0f, 1.0f));
  aCharacter->setDistancePrecision (btScalar(THE_DIST_PRECISION * aLenScale));
  aCharacter->setMaxPenetrationDepth (btScalar(myCapsMaxPenetr));
  aCharacter->setFallSpeed (btScalar(55.0 * aLenScale)); // terminal velocity of a sky diver in m/s
  aCharacter->setJumpSpeed (btScalar(10.0 * aLenScale));
  myAction.reset (aCharacter);
  SetGravity (myDefGravity);
  SetFallSpeed (myDefFallSpeed);
  if (aWorld != NULL)
  {
    addToWorld (aWorld);
  }
#endif
}

// =======================================================================
// function : addToWorld
// purpose  :
// =======================================================================
void Dynamics_Character::SetCustomConvexShape (const Handle(Dynamics_CollisionShape)& theShape)
{
  myCustomShape = theShape;
  if (!myCustomShape.IsNull())
  {
  #ifdef HAVE_BULLET
    const btConvexShape* aConvexShape = dynamic_cast<const btConvexShape*> (myCustomShape->CollisionShape());
    if (aConvexShape == NULL)
    {
      throw Standard_ProgramError ("Dynamics_Character::SetCustomConvexShape() - non-convex shape is set");
    }
  #endif
  }
  init();
}

// =======================================================================
// function : addToWorld
// purpose  :
// =======================================================================
void Dynamics_Character::addToWorld (Dynamics_World* theWorld)
{
  removeFromWorld();
#ifdef HAVE_BULLET
  myWorld = theWorld;
  if (myWorld != NULL && myBody.get() != NULL)
  {
    btDiscreteDynamicsWorld* aWorld = myWorld->myDynamicsWorld.get();
    aWorld->addCollisionObject (myBody.get(), btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
    aWorld->addAction (myAction.get());
  }
#else
  (void )theWorld;
#endif
}

// =======================================================================
// function : SetEyePosition
// purpose  :
// =======================================================================
void Dynamics_Character::SetEyePosition (const gp_Pnt&  theEyePosition,
                                            const Bnd_Box& theSceneBndBox)
{
  myVelocity.SetCoord (0.0, 0.0, 0.0);
  const double aShift = myCapsHalfHeight + myCapsMaxPenetr * 0.5 - myHumanHeight * ProportionalHumanEyeShift();
  gp_XYZ aPos (theEyePosition.XYZ());
  aPos.SetZ (theEyePosition.Z() - aShift);
  if (!theSceneBndBox.IsVoid())
  {
    aPos.SetZ (Max (aPos.Z(), theSceneBndBox.CornerMin().Z()));
  }
  SetWorldPosition (aPos);
}

// =======================================================================
// function : SetWorldPosition
// purpose  :
// =======================================================================
void Dynamics_Character::SetWorldPosition (const gp_Pnt& thePosition)
{
  myVelocity.SetCoord (0.0, 0.0, 0.0);
#ifdef HAVE_BULLET
  Dynamics_CharacterController* aCharacter = dynamic_cast<Dynamics_CharacterController* >(myAction.get());
  aCharacter->ResetStable();

  btPairCachingGhostObject* aGhostObject = dynamic_cast<btPairCachingGhostObject* >(myBody.get());
  btTransform aStartTrsf;
  aStartTrsf.setIdentity();
  aStartTrsf.setOrigin (btVector3 (btScalar (thePosition.X()), btScalar(thePosition.Y()), btScalar(thePosition.Z())));
  aGhostObject->setWorldTransform (aStartTrsf);
#else
  (void )thePosition;
#endif
}

// =======================================================================
// function : SetGravity
// purpose  :
// =======================================================================
void Dynamics_Character::SetGravity (const gp_Vec& theGravity)
{
  myGravity = theGravity;
#ifdef HAVE_BULLET
  Dynamics_CharacterController* aCharacter = dynamic_cast<Dynamics_CharacterController* >(myAction.get());
  aCharacter->setGravity (btVector3 (btScalar(theGravity.X()), btScalar(theGravity.Y()), btScalar(theGravity.Z())));
#endif
}

// =======================================================================
// function : SetFallSpeed
// purpose  :
// =======================================================================
void Dynamics_Character::SetFallSpeed (double theSpeed)
{
  myFallSpeed = theSpeed;
#ifdef HAVE_BULLET
  Dynamics_CharacterController* aCharacter = dynamic_cast<Dynamics_CharacterController* >(myAction.get());
  aCharacter->setFallSpeed (btScalar(theSpeed));
#endif
}

// =======================================================================
// function : Walk
// purpose  :
// =======================================================================
void Dynamics_Character::Walk (const gp_Vec& theMoveVec)
{
  myVelocity.SetCoord (0.0, 0.0, 0.0);
#ifdef HAVE_BULLET
  Dynamics_CharacterController* aCharacter = dynamic_cast<Dynamics_CharacterController* >(myAction.get());
  aCharacter->setWalkDirection (btVector3 (btScalar(theMoveVec.X()), btScalar(theMoveVec.Y()), btScalar(theMoveVec.Z())));
#else
  (void )theMoveVec;
#endif
}

// =======================================================================
// function : Walk
// purpose  :
// =======================================================================
void Dynamics_Character::Walk (const gp_Vec& theSpeedVec,
                                  const double  theDurationSec)
{
  myVelocity = theSpeedVec;
#ifdef HAVE_BULLET
  Dynamics_CharacterController* aCharacter = dynamic_cast<Dynamics_CharacterController* >(myAction.get());
  aCharacter->setVelocityForTimeInterval (btVector3 (btScalar(theSpeedVec.X()), btScalar(theSpeedVec.Y()), btScalar(theSpeedVec.Z())), btScalar(theDurationSec));
#else
  (void )theDurationSec;
#endif
}

// =======================================================================
// function : Jump
// purpose  :
// =======================================================================
void Dynamics_Character::Jump()
{
#ifdef HAVE_BULLET
  Dynamics_CharacterController* aCharacter = dynamic_cast<Dynamics_CharacterController* >(myAction.get());
  aCharacter->ResetStable();
  aCharacter->jump (btVector3 (0, 0, 0));
#endif
}
