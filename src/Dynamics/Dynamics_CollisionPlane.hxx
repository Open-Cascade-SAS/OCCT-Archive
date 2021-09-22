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

#ifndef _Dynamics_CollisionPlane_HeaderFile
#define _Dynamics_CollisionPlane_HeaderFile

#include <Dynamics_CollisionShape.hxx>

class gp_Pln;

//! Infinite collision plane.
class Dynamics_CollisionPlane : public Dynamics_CollisionShape
{
  DEFINE_STANDARD_RTTIEXT(Dynamics_CollisionPlane, Dynamics_CollisionShape)
public:

  //! Main constructor.
  Dynamics_CollisionPlane (const gp_Pln& thePlane)
  {
    InitPlane (thePlane);
  }

  //! Initialize the collision plane.
  Standard_EXPORT void InitPlane (const gp_Pln& thePlane);

};

#endif // _Dynamics_CollisionPlane_HeaderFile
