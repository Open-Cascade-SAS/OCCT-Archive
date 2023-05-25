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

#ifndef _XCAFAnimObjects_CustomOperation_HeaderFile
#define _XCAFAnimObjects_CustomOperation_HeaderFile

#include <XCAFAnimObjects_Operation.hxx>
#include <NCollection_Array2.hxx>
#include <TCollection_AsciiString.hxx>

//! 
class XCAFAnimObjects_CustomOperation : public XCAFAnimObjects_Operation
{
public:

  //! 
  Standard_EXPORT XCAFAnimObjects_CustomOperation(const NCollection_Array1<double>& thePresentation,
                                                  const TCollection_AsciiString& theCustomTypeName);

  //! 
  Standard_EXPORT XCAFAnimObjects_CustomOperation(const NCollection_Array2<double>& thePresentation,
                                                  const NCollection_Array1<double>& theTimeStamps,
                                                  const TCollection_AsciiString& theCustomTypeName);

  //! 
  XCAFAnimObjects_OperationType GetType() const Standard_OVERRIDE { return XCAFAnimObjects_OperationType_Custom; }

  //! 
  NCollection_Array2<double> GeneralPresentation() const Standard_OVERRIDE { return myPresentation; }

  //! 
  TCollection_AsciiString GetTypeName() const Standard_OVERRIDE { return myTypeName; }

  //! 
  const NCollection_Array2<double>& CustomPresentation() const { return myPresentation; }

private:

  TCollection_AsciiString myTypeName; //!< 
  NCollection_Array2<double> myPresentation; //!< 
};

#endif // _XCAFAnimObjects_CustomOperation_HeaderFile
