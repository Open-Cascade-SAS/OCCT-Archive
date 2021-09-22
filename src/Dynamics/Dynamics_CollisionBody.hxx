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

#ifndef _Dynamics_CollisionBody_HeaderFile
#define _Dynamics_CollisionBody_HeaderFile

#include <gp_Trsf.hxx>
#include <NCollection_Vector.hxx>
#include <Standard_Transient.hxx>

#include <memory>

class Dynamics_CollisionShape;
class Dynamics_Constraint;
class Dynamics_MotionState;
class Dynamics_World;

class btActionInterface;
class btCollisionObject;
class btCollisionShape;
class btTransform;

//! Class defining collision body.
class Dynamics_CollisionBody : public Standard_Transient
{
  friend class Dynamics_Constraint;
  friend class Dynamics_World;
  DEFINE_STANDARD_RTTIEXT(Dynamics_CollisionBody, Standard_Transient)
public:

  //! Empty constructor.
  Standard_EXPORT Dynamics_CollisionBody();

  //! Constructor.
  Standard_EXPORT Dynamics_CollisionBody (const Handle(Dynamics_CollisionShape)& theShape);

  //! Destructor, automatically removes the body from the world.
  Standard_EXPORT virtual ~Dynamics_CollisionBody();

  //! Return the world.
  Dynamics_World* DynamicsWorld() { return myWorld; }

  //! Return the shape.
  const Handle(Dynamics_CollisionShape)& Shape() const { return myShape; }

  //! Assign new shape.
  Standard_EXPORT void SetShape (const Handle(Dynamics_CollisionShape)& theShape,
                                 const gp_Trsf& theTrsf = gp_Trsf());

  //! Assign new shape.
  Standard_EXPORT void SetShape (const Handle(Dynamics_CollisionShape)& theShape,
                                 const Handle(Dynamics_MotionState)& theMotionState,
                                 const float theMass);

  //! Return motion state.
  const Handle(Dynamics_MotionState)& MotionState() const { return myMotionState; }

  //! Return constraint.
  const Handle(Dynamics_Constraint)& Constraint() const { return myConstraint; }

  //! Return the position (origin) in the world.
  Standard_EXPORT gp_Pnt WorldPosition() const;

  //! Return the location in the world.
  Standard_EXPORT gp_Trsf WorldLocation() const;

  //! Return the location in the world.
  Standard_EXPORT void SetWorldLocation (const gp_Trsf& theLoc);

  //! Return the location in the world.
  Standard_EXPORT gp_Trsf WorldMotionLocation() const;

  //! Alias for btRigidBody::applyCentralImpulse().
  Standard_EXPORT void ApplyCentralForce  (const gp_XYZ& theForce);

  //! Alias for btRigidBody::applyCentralImpulse().
  Standard_EXPORT void ApplyCentralImpulse(const gp_XYZ& theImpulse);

  //! Alias for btRigidBody::applyForce().
  Standard_EXPORT void ApplyForce  (const gp_XYZ& theForce, const gp_Pnt& theRelPos);

  //! Alias for btRigidBody::applyTorque().
  Standard_EXPORT void ApplyTorque (const gp_XYZ& theTorque);

  //! Alias for btRigidBody::applyImpulse().
  Standard_EXPORT void ApplyImpulse(const gp_XYZ& theImpulse, const gp_Pnt& theRelPos);

  //! Alias for btRigidBody::applyTorqueImpulse().
  Standard_EXPORT void ApplyTorqueImpulse (const gp_XYZ& theTorque);

public:

  //! Return collision body.
  btCollisionObject* collisionBody() { return myBody.get(); }

protected:

  //! Add body to the world.
  Standard_EXPORT virtual void addToWorld (Dynamics_World* theWorld);

  //! Remove body from the world.
  Standard_EXPORT virtual void removeFromWorld();

  //! Release the memory.
  Standard_EXPORT void clearMain();

protected:

  Handle(Dynamics_CollisionShape)    myShape;
  Handle(Dynamics_MotionState)       myMotionState;
  Handle(Dynamics_Constraint)        myConstraint;
  std::shared_ptr<btActionInterface> myAction;
  std::shared_ptr<btCollisionObject> myBody;
  Dynamics_World*                    myWorld;

};

#endif // _Dynamics_CollisionBody_HeaderFile
