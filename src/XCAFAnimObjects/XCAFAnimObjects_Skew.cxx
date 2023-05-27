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

#include <Message.hxx>

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

//=======================================================================
//function : XCAFAnimObjects_Skew
//purpose  :
//=======================================================================
XCAFAnimObjects_Skew::XCAFAnimObjects_Skew(const NCollection_Array2<double>& theGeneralPresentation,
                                           const NCollection_Array1<double>& theTimeStamps) :
  XCAFAnimObjects_Operation(false),
  mySkewPresentation(1, theGeneralPresentation.RowLength())
{
  if (theGeneralPresentation.ColLength() != 7)
  {
    Message::SendWarning() << "Warning: XCAFAnimObjects_Skew: Incorrect Skew general presentation";
    return;
  }
  for (int aRowInd = 1; aRowInd <= theGeneralPresentation.RowLength(); aRowInd++)
  {
    Skew aSkew{ theGeneralPresentation.Value(aRowInd, 1),
               {theGeneralPresentation.Value(aRowInd, 2),
               theGeneralPresentation.Value(aRowInd, 3),
               theGeneralPresentation.Value(aRowInd, 4) },
               {theGeneralPresentation.Value(aRowInd, 5),
               theGeneralPresentation.Value(aRowInd, 6),
               theGeneralPresentation.Value(aRowInd, 7)} };
    mySkewPresentation.SetValue(aRowInd, aSkew);
  }
}

//=======================================================================
//function : XCAFAnimObjects_Skew
//purpose  :
//=======================================================================
XCAFAnimObjects_Skew::XCAFAnimObjects_Skew(const Handle(XCAFAnimObjects_Skew)& theOperation) :
  XCAFAnimObjects_Operation(theOperation),
  mySkewPresentation(theOperation->mySkewPresentation)
{}

//=======================================================================
//function : GeneralPresentation
//purpose  :
//=======================================================================
NCollection_Array2<double> XCAFAnimObjects_Skew::GeneralPresentation() const
{
  NCollection_Array2<double> aRes(1, mySkewPresentation.Length(), 1, 7);
  for (int aRowInd = 1; aRowInd <= mySkewPresentation.Length(); aRowInd++)
  {
    const Skew& aSkew = mySkewPresentation.Value(aRowInd);

    aRes.SetValue(aRowInd, 1, aSkew.Angle);
    aRes.SetValue(aRowInd, 2, aSkew.Axis1.X());
    aRes.SetValue(aRowInd, 3, aSkew.Axis1.Y());
    aRes.SetValue(aRowInd, 4, aSkew.Axis1.Z());
    aRes.SetValue(aRowInd, 5, aSkew.Axis2.X());
    aRes.SetValue(aRowInd, 6, aSkew.Axis2.Y());
    aRes.SetValue(aRowInd, 7, aSkew.Axis2.Z());
  }
}