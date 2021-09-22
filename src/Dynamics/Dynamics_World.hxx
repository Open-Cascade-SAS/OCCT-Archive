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

#ifndef _Dynamics_World_HeaderFile
#define _Dynamics_World_HeaderFile

#include <Dynamics_CollisionBody.hxx>
#include <Dynamics_Character.hxx>

#include <Bnd_Box.hxx>
#include <NCollection_IndexedMap.hxx>
#include <OSD_Timer.hxx>

class btBroadphaseInterface;
class btCollisionDispatcher;
class btDefaultCollisionConfiguration;
class btDiscreteDynamicsWorld;
class btGhostPairCallback;
class btSequentialImpulseConstraintSolver;

class Dynamics_DebugDrawer;
class Dynamics_Parameters;
class AIS_InteractiveContext;

//! Physics engine - world holder.
class Dynamics_World : public Standard_Transient
{
  friend class Dynamics_CollisionBody;
  friend class Dynamics_Constraint;
  friend class Dynamics_Character;
  DEFINE_STANDARD_RTTIEXT(Dynamics_World, Standard_Transient)
public:

  //! Main constructor.
  Standard_EXPORT Dynamics_World (const Handle(Dynamics_Parameters)& theDefaultStyle = Handle(Dynamics_Parameters)());

  //! Destructor.
  Standard_EXPORT virtual ~Dynamics_World();

  //! Add collision body to the world.
  Standard_EXPORT void AddCollisionBody (const Handle(Dynamics_CollisionBody)& theBody);

  //! Remove collision body from the world.
  Standard_EXPORT void RemoveCollisionBody (const Handle(Dynamics_CollisionBody)& theBody);

  //! Add constraint to the world.
  Standard_EXPORT void AddConstraint (const Handle(Dynamics_Constraint)& theConstraint);

  //! Remove constraint from the world.
  Standard_EXPORT void RemoveConstraint (const Handle(Dynamics_Constraint)& theConstraint);

  //! Return the character object.
  const Handle(Dynamics_Character)& Character() const { return myCharacter; }

  //! Return scene boundaries.
  const Bnd_Box& SceneBoundaries() const { return mySceneBox; }

  //! Define scene boundaries.
  Standard_EXPORT void SetSceneBoundaries (const Bnd_Box& theBndBox);

  //! Return length unit scale factor (as scale factor for meters); 1.0 by default.
  double LengthUnitScale() const { return myLengthUnitScale; }

  //! Set length unit scale factor.
  Standard_EXPORT void SetLengthUnitScale (double theScale);

  //! Update first-person capsule size basing on Drawer parameters.
  Standard_EXPORT void UpdateFirstPersonSize();

  //! Return time step in seconds for computing dynamics within discrete math; 1/120 by default.
  double FixedTimeStep() const { return myFixedTimeStep; }

  //! Setup time step in seconds.
  void SetFixedTimeStep (double theStepSeconds) { myFixedTimeStep = theStepSeconds; }

  //! Return maximum number of sub steps in case if time increment is greater than fixed time step.
  //! 20 by default, which means that within default 1/120 fixed time step
  //! the playback should not be lower than 6 FPS for proper computations.
  int MaximumSubSteps() const { return myNbMaxSubTicks; }

  //! Setup maximum number of sub steps in case if time increment is greater than fixed time step.
  void SetMaximumSubSteps (int theNbMaxSubTicks) { myNbMaxSubTicks = theNbMaxSubTicks; }

  //! Return TRUE if world timer is in paused state; TRUE by default.
  bool IsPaused() const { return !myWorldTimer.IsStarted(); }

  //! Start/pause world timer.
  //! Note that StepSimulation() will automatically resume timer!
  Standard_EXPORT void SetPaused (bool theToPause);

  //! Proceed the simulation over Time step units.
  //! Note that if world timer was in paused state, it will be automatically resumed!
  //! @param theMaxSubSteps maximum number of sub-steps in case if time increment is greater then fixed time step
  //! @param theCtx interactive context for drawing debug information
  //! @return TRUE if simulation was updated
  Standard_EXPORT bool StepSimulation (const Handle(AIS_InteractiveContext)& theCtx);

  //! Return TRUE if debug drawer is enabled.
  bool ToEnableDebugDrawer() const { return myDebugDrawer.get() != NULL; }

  //! Set if debug drawer should be enabled/disabled.
  Standard_EXPORT bool SetEnableDebugDrawer (bool theToDebug,
                                             AIS_InteractiveContext* theCtx);

  //! Return the number of registered rigid bodies.
  int NbRigidBodies() const { return myNbRigidBodies; }

protected:

  std::shared_ptr<btGhostPairCallback>                   myGhostPairCallback;
  std::shared_ptr<btDefaultCollisionConfiguration>       myCollisionConfiguration;
  std::shared_ptr<btCollisionDispatcher>                 myDispatcher;
  std::shared_ptr<btBroadphaseInterface>                 myOverlappingPairCache;
  std::shared_ptr<btSequentialImpulseConstraintSolver>   mySolver;
  std::shared_ptr<Dynamics_DebugDrawer>                  myDebugDrawer;
  std::shared_ptr<btDiscreteDynamicsWorld>               myDynamicsWorld;

  Handle(Dynamics_Parameters)                            myDefaultDrawer;     //!< default attributes
  Handle(Dynamics_Character)                             myCharacter;
  Handle(Dynamics_CollisionBody)                         myGround;
  NCollection_IndexedMap<Handle(Dynamics_CollisionBody)> myCollisionBodies;
  NCollection_IndexedMap<Handle(Dynamics_Constraint)>    myConstraints;
  Bnd_Box                                                mySceneBox;
  OSD_Timer                                              myWorldTimer;
  Standard_Real                                          myLengthUnitScale;
  Standard_Real                                          myLastWorldTick;
  Standard_Real                                          myFixedTimeStep;
  Standard_Integer                                       myNbMaxSubTicks;
  Standard_Integer                                       myNbRigidBodies;

};

#endif // _Dynamics_World_HeaderFile
