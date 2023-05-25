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

#include "XCAFAnimObjects_CustomOperation.hxx"

//=======================================================================
//function : XCAFAnimObjects_CustomOperation
//purpose  :
//=======================================================================
XCAFAnimObjects_CustomOperation::XCAFAnimObjects_CustomOperation(const NCollection_Array1<double>& thePresentation,
                                                                 const TCollection_AsciiString& theCustomTypeName) :
  XCAFAnimObjects_Operation(false),
  myTypeName(theCustomTypeName),
  myPresentation(1, 1, thePresentation.Lower(), thePresentation.Upper())
{
  for (int anInd = thePresentation.Lower(); anInd <= thePresentation.Upper(); anInd++)
  {
    myPresentation.SetValue(1, anInd, thePresentation.Value(anInd));
  }
}

//=======================================================================
//function : XCAFAnimObjects_CustomOperation
//purpose  :
//=======================================================================
XCAFAnimObjects_CustomOperation::XCAFAnimObjects_CustomOperation(const NCollection_Array2<double>& thePresentation,
                                                                 const NCollection_Array1<double>& theTimeStamps,
                                                                 const TCollection_AsciiString& theCustomTypeName) :
  XCAFAnimObjects_Operation(theTimeStamps),
  myTypeName(theCustomTypeName),
  myPresentation(thePresentation)
{}
