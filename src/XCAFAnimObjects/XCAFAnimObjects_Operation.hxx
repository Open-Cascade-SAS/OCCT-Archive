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

#ifndef _XCAFAnimObjects_Operation_HeaderFile
#define _XCAFAnimObjects_Operation_HeaderFile

#include <Standard_Type.hxx>
#include <Standard_Transient.hxx>
#include <NCollection_Array1.hxx>
#include <NCollection_Array2.hxx>
#include <TCollection_AsciiString.hxx>
#include <XCAFAnimObjects_OperationType.hxx>

//! 
class XCAFAnimObjects_Operation : public Standard_Transient
{
public:

  //! 
  Standard_EXPORT XCAFAnimObjects_Operation(const bool theIsInverse = false);

  //! 
  Standard_EXPORT XCAFAnimObjects_Operation(const NCollection_Array1<double>& theTimeStamps,
                                            const bool theIsInverse = false);

  //!
  Standard_EXPORT XCAFAnimObjects_Operation(const Handle(XCAFAnimObjects_Operation)& theOperation);

  //! 
  const NCollection_Array1<double>& TimeStamps() const { return myTimeStamps; }

  //! 
  bool HasTimeStamps() const { return myTimeStamps.IsEmpty(); }

  //! 
  Standard_EXPORT virtual XCAFAnimObjects_OperationType GetType() const = 0;

  //! 
  Standard_EXPORT virtual TCollection_AsciiString GetTypeName() const = 0;

  //! 
  bool IsInverse() const { return myIsInverse; }

  //! 
  void SetInverse(const bool theIsInverse) { myIsInverse = theIsInverse; }

  //! 
  Standard_EXPORT virtual NCollection_Array2<double> GeneralPresentation() const = 0;

  DEFINE_STANDARD_RTTIEXT(XCAFAnimObjects_Operation, Standard_Transient)

private:
  bool myIsInverse; //!
  NCollection_Array1<double> myTimeStamps; //!< 
};

#endif // _XCAFAnimObjects_Operation_HeaderFile
