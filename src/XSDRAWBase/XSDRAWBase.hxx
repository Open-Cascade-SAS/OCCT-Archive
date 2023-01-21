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

#ifndef _XSDRAWBase_HeaderFile
#define _XSDRAWBase_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <XSControl_WorkSession.hxx>
#include <TCollection_AsciiString.hxx>

class TDocStd_Document;

//! Basic package to work functions of X-STEP (IFSelect & Co)
//! under control of DRAW
//!
//! Works with some "static" data : a SessionPilot (used to run)
//! with its WorkSession and Model and TransferReader, a
//! FinderProcess
class XSDRAWBase
{
  DEFINE_STANDARD_ALLOC
public:

  //! Returns the WorkSession defined in AddDraw
  //! It is from XSControl, it brings functionalities for Transfers
  Standard_EXPORT static Handle(XSControl_WorkSession)& Session();

  //!
  Standard_EXPORT static Standard_Real GetLengthUnit(const Handle(TDocStd_Document)& theDoc = nullptr);

  //!
  Standard_EXPORT static XSControl_WorkSessionMap& WorkSessionList();

  //!
  Standard_EXPORT static void CollectActiveWorkSessions(const Handle(XSControl_WorkSession)& theWS,
                                                        const TCollection_AsciiString& theName,
                                                        XSControl_WorkSessionMap& theMap);

  //!
  Standard_EXPORT static void CollectActiveWorkSessions(const TCollection_AsciiString& theName);
};

#endif // _XSDRAWBase_HeaderFile
