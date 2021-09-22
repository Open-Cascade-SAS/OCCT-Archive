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

#ifndef _Dynamics_CollisionCompoundShape_HeaderFile
#define _Dynamics_CollisionCompoundShape_HeaderFile

#include <NCollection_Array1.hxx>

#include <Dynamics_CollisionShape.hxx>

//! Define the collision body from triangulation data.
class Dynamics_CollisionCompoundShape : public Dynamics_CollisionShape
{
  DEFINE_STANDARD_RTTIEXT(Dynamics_CollisionCompoundShape, Dynamics_CollisionShape)
public:

  //! Constructor.
  Standard_EXPORT Dynamics_CollisionCompoundShape();

  //! Destructor.
  Standard_EXPORT virtual ~Dynamics_CollisionCompoundShape();

  //! Modify sub-shapes.
  NCollection_Array1<Handle(Dynamics_CollisionShape)>& ChangeSubShapes() { return mySubShapes; }

  //! Initialize the collision shape after modification of sub-shape
  Standard_EXPORT bool Init();

protected:

  NCollection_Array1<Handle(Dynamics_CollisionShape)> mySubShapes;

};

#endif // _Dynamics_CollisionCompoundShape_HeaderFile
