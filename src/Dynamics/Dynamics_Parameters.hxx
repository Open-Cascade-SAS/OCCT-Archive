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

#ifndef _Dynamics_Parameters_HeaderFile
#define _Dynamics_Parameters_HeaderFile

#include <Standard_Type.hxx>

//! Class defining global dynamics parameters.
class Dynamics_Parameters : public Standard_Transient
{
  DEFINE_STANDARD_RTTIEXT(Dynamics_Parameters, Standard_Transient)
public:

  //! Empty constructor.
  Standard_EXPORT Dynamics_Parameters();

  //! Return rigid body yaw constraint in degrees; -1 by default.
  //! -1 means no constraint.
  int RigidBodyYawConstraint() const { return myRigidBodyYaw; }

  //! Set rigid body yaw constraint in degrees.
  void SetRigidBodyYawConstraint (int theAngle) { myRigidBodyYaw = theAngle; }

  //! Return rigid body pitch constraint in degrees; 15 degrees by default.
  //! -1 means no constraint.
  int RigidBodyPitchConstraint() const { return myRigidBodyPitch; }

  //! Set rigid body pitch constraint in degrees.
  void SetRigidBodyPitchConstraint (int theAngle) { myRigidBodyPitch = theAngle; }

  //! Return rigid body roll constraint in degrees; 15 degrees by default.
  //! -1 means no constraint.
  int RigidBodyRollConstraint() const { return myRigidBodyRoll; }

  //! Set rigid body roll constraint in degrees.
  void SetRigidBodyRollConstraint (int theAngle) { myRigidBodyRoll = theAngle; }

  //! Return height of first person capsule, in meters; 1.75 by default.
  float FirstPersonHeight() const { return myFirstPersonHeight; }

  //! Set height of first person capsule.
  void SetFirstPersonHeight (float theHeight) { myFirstPersonHeight = theHeight; }

  //! Return width (diameter) of first person capsule, in meters; 0.46 by default.
  float FirstPersonWidth() const { return myFirstPersonWidth; }

  //! Set width of first person capsule.
  void SetFirstPersonWidth (float theWidth) { myFirstPersonWidth = theWidth; }

  //! Return step height of first person, in meters; 0.35 by default.
  float FirstPersonStepHeight() const { return myFirstPersonStepHeight; }

  //! Set step height of first person.
  void SetFirstPersonStepHeight (float theStepHeight) { myFirstPersonStepHeight = theStepHeight; }

protected:

  int   myRigidBodyYaw;          //!< rigid body yaw   constraint
  int   myRigidBodyPitch;        //!< rigid body pitch constraint
  int   myRigidBodyRoll;         //!< rigid body roll  constraint
  float myFirstPersonHeight;     //!< height of first person capsule, in meters
  float myFirstPersonWidth;      //!< width (diameter) of first person capsule, in meters
  float myFirstPersonStepHeight; //!< step height of first person, in meters

};

#endif // _Dynamics_Parameters_HeaderFile
