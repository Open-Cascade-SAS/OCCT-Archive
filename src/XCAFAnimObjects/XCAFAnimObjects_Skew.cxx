// Copyright (c) 2023 OPEN CASCADE SAS
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

#include "XCAFAnimObjects_Skew.hxx"

//=======================================================================
//function : XCAFAnimObjects_Skew
//purpose  :
//=======================================================================
XCAFAnimObjects_Skew::XCAFAnimObjects_Skew(const Skew& theSkew) :
  XCAFAnimObjects_Operation(false),
  mySkewPresentation(1, 1)
{
  mySkewPresentation.SetValue(1, theSkew);
}

//=======================================================================
//function : XCAFAnimObjects_Skew
//purpose  :
//=======================================================================
XCAFAnimObjects_Skew::XCAFAnimObjects_Skew(const NCollection_Array1<Skew>& theSkew,
                                           const NCollection_Array1<double>& theTimeStamps) :
  XCAFAnimObjects_Operation(theTimeStamps),
  mySkewPresentation(theSkew)
{}
