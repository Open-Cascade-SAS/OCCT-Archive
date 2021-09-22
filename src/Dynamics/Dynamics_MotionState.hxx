// Copyright (c) 2018-2021 OPEN CASCADE SAS
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

#ifndef _Dynamics_MotionState_HeaderFile
#define _Dynamics_MotionState_HeaderFile

#include <gp_Trsf.hxx>
#include <Standard_Transient.hxx>
#include <Standard_Type.hxx>

class btMotionState;
class btTransform;

//! Define the motion state.
class Dynamics_MotionState : public Standard_Transient
{
  DEFINE_STANDARD_RTTIEXT(Dynamics_MotionState, Standard_Transient)
public:

  //! Convert transformation definition from OCCT to Bullet structure.
  Standard_EXPORT static void TrsfBtFromGp (btTransform& theTrsf, const gp_Trsf& theLoc);

  //! Convert transformation definition from Bullet to OCCT structure.
  Standard_EXPORT static void TrsfGpFromBt (gp_Trsf& theLoc, const btTransform& theTrsf);
public:

  //! Empty constructor.
  Standard_EXPORT Dynamics_MotionState();

  //! Destructor.
  Standard_EXPORT virtual ~Dynamics_MotionState();

  //! Create a proxy motion state.
  //! @param theStartTrans initial transformation
  //! @param theCenterOfMassOffset offset to the center of mass
  //! @param thePrsOffset optional transformation from collision shape to presentation shape
  Standard_EXPORT void InitProxy (const gp_Trsf& theStartTrans,
                                  const gp_Trsf& theCenterOfMassOffset,
                                  const gp_Trsf& thePrsOffset = gp_Trsf());

  //! Create a default motion state.
  Standard_EXPORT void InitDefault (const btTransform& theStartTrans,
                                    const btTransform& theCenterOfMassOffset);

  //! Return true if motion state is defined.
  bool IsValid() const { return myMotionState != NULL; }

  //! Return motion state pointer.
  btMotionState* MotionState() const { return myMotionState; }

  //! Option displaying center of mass; FALSE by default.
  bool ToDisplaceCenterOfMass() const { return myToDisplaceCenterOfMass; }

  //! Set option displaying center of mass.
  void SetDisplaceCenterOfMass (bool theToDisplace) { myToDisplaceCenterOfMass = theToDisplace; }

protected:

  //! Virtual callback.
  Standard_EXPORT virtual void setWorldTransform (const gp_Trsf& theTrsf);

  //! Release memory.
  Standard_EXPORT void releaseMotionState();

protected:
  //! Proxy btMotionState implementation.
  struct Dynamics_MotionStateProxy;

protected:

  btMotionState* myMotionState;
  bool           myToDisplaceCenterOfMass;

};

#endif // _Dynamics_MotionState_HeaderFile
