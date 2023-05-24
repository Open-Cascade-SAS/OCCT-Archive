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

#include "XCAFAnimObjects_Transform.hxx"

//=======================================================================
//function : XCAFAnimObjects_Transform
//purpose  :
//=======================================================================
XCAFAnimObjects_Transform::XCAFAnimObjects_Transform(const gp_GTrsf& theTransform) :
  XCAFAnimObjects_Operation(false),
  myTransformPresentation(1, 1)
{
  myTransformPresentation.SetValue(1, theTransform);
}

//=======================================================================
//function : XCAFAnimObjects_Transform
//purpose  :
//=======================================================================
XCAFAnimObjects_Transform::XCAFAnimObjects_Transform(const NCollection_Array1<gp_GTrsf>& theTransform,
                                                     const NCollection_Array1<double>& theTimeStamps) :
  XCAFAnimObjects_Operation(theTimeStamps),
  myTransformPresentation(theTransform)
{}
