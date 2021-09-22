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

#ifndef _Dynamics_Constraint_HeaderFile
#define _Dynamics_Constraint_HeaderFile

#include <gp_Trsf.hxx>
#include <Standard_Transient.hxx>
#include <Standard_Type.hxx>

class btTypedConstraint;
class Dynamics_CollisionBody;
class Dynamics_World;

//! Define the motion state.
class Dynamics_Constraint : public Standard_Transient
{
  friend class Dynamics_World;
  DEFINE_STANDARD_RTTIEXT(Dynamics_Constraint, Standard_Transient)
public:

  //! Empty constructor.
  Standard_EXPORT Dynamics_Constraint();

  //! Destructor.
  Standard_EXPORT virtual ~Dynamics_Constraint();

  //! Return true if Constraint is defined.
  bool IsValid() const { return myConstraint != NULL; }

  //! Return Constraint pointer.
  btTypedConstraint* TypedConstraint() const { return myConstraint; }

  Standard_EXPORT void CreateGeneric6Dof (const Handle(Dynamics_CollisionBody)& theBody,
                                          const gp_XYZ& theLinearLower,
                                          const gp_XYZ& theLinearUpper,
                                          const gp_XYZ& theAngularLower,
                                          const gp_XYZ& theAngularUpper);

  //! Return the world.
  Dynamics_World* DynamicsWorld() { return myWorld; }

  //! Return linear limits per axis.
  //! Upper value below Lower means that there is NO limit.
  Standard_EXPORT bool LinearLimits (gp_XYZ& theLower, gp_XYZ& theUpper) const;

  //! Set linear limits per axis (in radians).
  Standard_EXPORT void SetLinearLimits (const gp_XYZ& theLower, const gp_XYZ& theUpper);

  //! Return angular limits per axis (in radians).
  //! Upper value below Lower means that there is NO limit.
  Standard_EXPORT bool AngularLimits(gp_XYZ& theLower, gp_XYZ& theUpper) const;

  //! Set angular limits per axis (in radians).
  Standard_EXPORT void SetAngularLimits (const gp_XYZ& theLower, const gp_XYZ& theUpper);

protected:

  //! Release memory.
  Standard_EXPORT void releaseConstraint();

  //! Add body to the world.
  Standard_EXPORT void addToWorld (Dynamics_World* theWorld);

  //! Remove body from the world.
  Standard_EXPORT void removeFromWorld();

protected:

  Dynamics_World* myWorld;
  btTypedConstraint* myConstraint;

};

#endif // _Dynamics_Constraint_HeaderFile
