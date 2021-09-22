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

#ifndef _Dynamics_Character_HeaderFile
#define _Dynamics_Character_HeaderFile

#include <Dynamics_CollisionBody.hxx>
#include <Dynamics_CharacterState.hxx>

class Bnd_Box;
class gp_Vec;

//! Class defining character object.
class Dynamics_Character : public Dynamics_CollisionBody
{
  friend class Dynamics_World;
  DEFINE_STANDARD_RTTIEXT(Dynamics_Character, Dynamics_CollisionBody)
public:

  //! Return the average human height.
  static double AverageHumanHeight() { return 1.75; }

  //! Compute human width from the human height using average proportions.
  static double ProportionalHumanWidth (double theHumanHeight) { return theHumanHeight * 0.265; }

  //! Compute human step height from the human height using average proportions.
  static double ProportionalHumanStepHeight (double theHumanHeight) { return theHumanHeight * 0.2; }

  //! Return the average human eye level shift (EyeLevel = Height - EyeShift), ~1/16.
  static double ProportionalHumanEyeShift() { return 0.0625; }

public:

  //! Main constructor.
  Standard_EXPORT Dynamics_Character (double theHumanHeight = 1.75);

  //! Destructor.
  Standard_EXPORT virtual ~Dynamics_Character();

  //! Initialize the character using average proportions for specified human height.
  void Init (double theHumanHeight)
  {
    Init (theHumanHeight, ProportionalHumanWidth (theHumanHeight), ProportionalHumanStepHeight (theHumanHeight));
  }

  //! Initialize the character.
  Standard_EXPORT void Init (double theHumanHeight,
                             double theHumanWidth,
                             double theStepHeight);

  //! Setup custom convex shape instead of capsule.
  Standard_EXPORT void SetCustomConvexShape (const Handle(Dynamics_CollisionShape)& theShape);

  //! Wrap to specified position.
  Standard_EXPORT void SetEyePosition (const gp_Pnt&  theEyePosition,
                                       const Bnd_Box& theSceneBndBox);

  //! Return the eye location.
  gp_Pnt EyePosition() const
  {
    const double aShift = myCapsHalfHeight + myCapsMaxPenetr * 0.5 - myHumanHeight * ProportionalHumanEyeShift();
    const gp_Pnt aPos = WorldPosition();
    return gp_Pnt (aPos.X(), aPos.Y(), aPos.Z() + aShift);
  }

  //! Wrap to specified position.
  Standard_EXPORT void SetWorldPosition (const gp_Pnt& thePosition);

  //! Return the ground location.
  gp_Pnt GroundPosition() const
  {
    const double aShift = -myCapsHalfHeight + myCapsMaxPenetr * 0.5;
    const gp_Pnt aPos = WorldPosition();
    return gp_Pnt (aPos.X(), aPos.Y(), aPos.Z() + aShift);
  }

  //! Return the default gravity vector.
  const gp_Vec& DefaultGravity() const { return myDefGravity; }

  //! Return the gravity vector.
  const gp_Vec& Gravity() const { return myGravity; }

  //! Setup new gravity.
  Standard_EXPORT void SetGravity (const gp_Vec& theGravity);

  //! Return the default fall speed limit.
  double DefaultFallSpeed() const { return myDefFallSpeed; }

  //! Return the fall speed limit.
  double FallSpeed() const { return myFallSpeed; }

  //! Setup fall speed limit.
  Standard_EXPORT void SetFallSpeed (double theSpeed);

  //! Walk in specified direction and delta.
  Standard_EXPORT void Walk (const gp_Vec& theMoveVec);

  //! Walk in specified direction and delta.
  Standard_EXPORT void Walk (const gp_Vec& theSpeedVec,
                             const double  theDurationSec);

  //! Jump impulse.
  Standard_EXPORT void Jump();

  //! Return current state.
  Dynamics_CharacterState State() const { return myState; }

  //! Return current state.
  void SetState (Dynamics_CharacterState theState)
  {
    if (myState == theState)
    {
      return;
    }
    myState = theState;
    init();
  }

  //! Return character height.
  double EyeLevel() const
  {
    double aHumanHeight = myHumanHeight;
    if (myState == Dynamics_CharacterState_Crouch)
    {
      aHumanHeight = myHumanHeight * 0.5;
    }
    return aHumanHeight - myHumanHeight * ProportionalHumanEyeShift();
  }

  //! Return character height.
  double Height()     const { return myHumanHeight; }

  //! Return character width.
  double Width()      const { return myHumanWidth;  }

  //! Return character step height.
  double StepHeight() const { return myStepHeight; }

  //! Running speed ratio (relative to normal walk), 3 by default.
  double RunRatio() const { return myRunRatio; }

  //! Setup running speed ratio.
  void SetRunRatio (double theRatio) { myRunRatio = theRatio; }

  //! Crouching speed ratio (relative to normal walk), 0.5 by default.
  double CrouchRatio() const { return myCrouchRatio; }

  //! Crouching running speed ratio.
  void SetCrouchRatio (double theRatio) { myCrouchRatio = theRatio; }

  //! Return walking velocity.
  const gp_Vec& Velocity() const { return myVelocity; }

  //! Return capsule half height (offset from the local CS).
  double CapsuleHalfHeight() const { return myCapsHalfHeight; }

  //! Return allowed penetration.
  double CapsuleMaxPenetration() const { return myCapsMaxPenetr; }

  //! Return TRUE if character is added to the World.
  bool IsActive() const { return myWorld != nullptr; }

  //! Add/remove character to/from the World.
  void SetActive (Dynamics_World* theWorld)
  {
    if (myWorld == theWorld)
    {
      return;
    }
    if (theWorld != nullptr)
    {
      addToWorld (theWorld);
      init();
    }
    else
    {
      removeFromWorld();
    }
  }

protected:

  //! Add body to the world.
  Standard_EXPORT virtual void addToWorld (Dynamics_World* theWorld) Standard_OVERRIDE;

  //! Initialize the character.
  Standard_EXPORT void init();

protected:

  Handle(Dynamics_CollisionShape) myCustomShape;
  gp_Vec         myVelocity;
  gp_Vec         myDefGravity;     //!< default gravity
  gp_Vec         myGravity;        //!< current gravity
  Standard_Real  myDefFallSpeed;   //!< default maximum fall speed
  Standard_Real  myFallSpeed;      //!< maximum fall speed
  Standard_Real  myHumanHeight;    //!< character height
  Standard_Real  myHumanWidth;     //!< character width
  Standard_Real  myStepHeight;     //!< step height
  Standard_Real  myRunRatio;       //!< running   speed ratio (relative to normal walk)
  Standard_Real  myCrouchRatio;    //!< crouching speed ratio (relative to normal walk)
  Standard_Real  myCapsHalfHeight; //!< capsule half height (offset from the local CS)
  Standard_Real  myCapsMaxPenetr;  //!< allowed penetration
  Dynamics_CharacterState myState; //!< current character state

};

#endif // _Dynamics_Character_HeaderFile
