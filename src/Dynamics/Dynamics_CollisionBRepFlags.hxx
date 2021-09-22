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

#ifndef _Dynamics_CollisionBRepFlags_HeaderFile
#define _Dynamics_CollisionBRepFlags_HeaderFile

//! Dynamics_CollisionBRepShape initialization parameters.
enum Dynamics_CollisionBRepFlags
{
  Dynamics_CollisionBRepFlags_NONE       = 0x00,
  Dynamics_CollisionBRepFlags_Convex     = 0x02,
  Dynamics_CollisionBRepFlags_Single     = 0x04,
  Dynamics_CollisionBRepFlags_Compressed = 0x08,
  Dynamics_CollisionBRepFlags_BuildBVH   = 0x10,

  Dynamics_CollisionBRepFlags_DEFAULTS   = Dynamics_CollisionBRepFlags_Compressed | Dynamics_CollisionBRepFlags_BuildBVH,
  Dynamics_CollisionBRepFlags_CONVEX_DEFAULTS = Dynamics_CollisionBRepFlags_DEFAULTS | Dynamics_CollisionBRepFlags_Single | Dynamics_CollisionBRepFlags_Convex,
};

#endif // _Dynamics_CollisionBRepFlags_HeaderFile
