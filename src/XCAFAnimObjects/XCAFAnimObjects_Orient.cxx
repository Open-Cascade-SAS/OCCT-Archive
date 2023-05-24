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

#include <XCAFAnimObjects_Orient.hxx>

//=======================================================================
//function : XCAFAnimObjects_Orient
//purpose  :
//=======================================================================
XCAFAnimObjects_Orient::XCAFAnimObjects_Orient(const gp_Quaternion& theOrient) :
  XCAFAnimObjects_Operation(false),
  myOrientPresentation(1,1)
{
  myOrientPresentation.SetValue(1, theOrient);
}

//=======================================================================
//function : XCAFAnimObjects_Orient
//purpose  :
//=======================================================================
XCAFAnimObjects_Orient::XCAFAnimObjects_Orient(const NCollection_Array1<gp_Quaternion>& theOrient,
                                               const NCollection_Array1<double>& theTimeStamps) :
  
  XCAFAnimObjects_Operation(theTimeStamps),
  myOrientPresentation(theOrient)
{}
