// Copyright (c) 2021 OPEN CASCADE SAS
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

#ifndef Draw_PntParams_HeaderFile
#define Draw_PntParams_HeaderFile

#include <Draw_Color.hxx>
#include <Draw_MarkerShape.hxx>

//! Draw parameters for points.
struct Draw_PntParams
{
public:
  Draw_Color       PntColor;
  Draw_MarkerShape PntMarker;

  Draw_PntParams()
  : PntColor   (Draw_rouge),
    PntMarker  (Draw_Plus)
  {}

};

#endif
