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

#ifndef _XCAFAnimObjects_AnimObject_HeaderFile
#define _XCAFAnimObjects_AnimObject_HeaderFile

#include <Standard_Type.hxx>
#include <Standard_Transient.hxx>
#include <NCollection_List.hxx>
#include <XCAFAnimObjects_InterpolationType.hxx>

class XCAFAnimObjects_Operation;

//!
class XCAFAnimObjects_AnimObject : public Standard_Transient
{
public:
  DEFINE_STANDARD_RTTIEXT(XCAFAnimObjects_AnimObject, Standard_Transient)

public:

  //! 
  Standard_EXPORT XCAFAnimObjects_AnimObject();

  //! 
  Standard_EXPORT XCAFAnimObjects_AnimObject(const Handle(XCAFAnimObjects_AnimObject)& theObject);

  //!
  void AppendNewOperation(const Handle(XCAFAnimObjects_Operation)& theOperation) { myOrderedOperations.Append(theOperation); }

  //!
  XCAFAnimObjects_InterpolationType GetInterpolationType() const { return myInterpolationType; }

  //!
  void SetInterpolationType(const XCAFAnimObjects_InterpolationType theType) { myInterpolationType = theType; }

  //! 
  const NCollection_List<Handle(XCAFAnimObjects_Operation)>& GetOrderedOperations() const { return myOrderedOperations; }

  //! 
  NCollection_List<Handle(XCAFAnimObjects_Operation)>& ChangeOrderedOperations() { return myOrderedOperations; }

private:

  XCAFAnimObjects_InterpolationType myInterpolationType;
  NCollection_List<Handle(XCAFAnimObjects_Operation)> myOrderedOperations;
};

#endif // _XCAFAnimObjects_AnimObject_HeaderFile
