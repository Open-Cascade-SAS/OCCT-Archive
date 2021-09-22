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

#ifndef _Dynamics_CollisionShape_HeaderFile
#define _Dynamics_CollisionShape_HeaderFile

#include <gp_Trsf.hxx>
#include <Standard_Transient.hxx>
#include <Standard_Type.hxx>

class btCollisionShape;

//! Define the collision shape.
class Dynamics_CollisionShape : public Standard_Transient
{
  friend class Dynamics_CollisionBody;
  friend class Dynamics_CollisionCompoundShape;
  DEFINE_STANDARD_RTTIEXT(Dynamics_CollisionShape, Standard_Transient)
public:

  //! Empty constructor.
  Standard_EXPORT Dynamics_CollisionShape();

  //! Constructor.
  Standard_EXPORT Dynamics_CollisionShape (btCollisionShape* theShape);

  //! Destructor.
  Standard_EXPORT virtual ~Dynamics_CollisionShape();

  //! Return true if shape object is defined.
  bool IsValid() const { return myShape != NULL; }

  //! Return collision shape pointer.
  btCollisionShape* CollisionShape() const { return myShape; }

  //! Return shape location.
  //! Default implementation always returns an identity location,
  //! because it is expected that location is put into Motion State instead.
  //! This method can be overridden for storing an initial location.
  virtual const gp_Trsf& Location() const
  {
    static const gp_Trsf THE_IDENT_LOC;
    return THE_IDENT_LOC;
  }

protected:

  //! Release memory.
  Standard_EXPORT void releaseShape();

protected:
  btCollisionShape* myShape;
};

#endif // _Dynamics_CollisionShape_HeaderFile
