// Created on: 1995-06-01
// Created by: Christian CAILLET
// Copyright (c) 1995-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
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

#ifndef _XSControl_WorkSession_HeaderFile
#define _XSControl_WorkSession_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <Message_ProgressRange.hxx>
#include <TCollection_AsciiString.hxx>
#include <NCollection_DataMap.hxx>

class DE_DataModel;

typedef NCollection_DataMap<TCollection_AsciiString, Handle(DE_WorkSession)> XSControl_WorkSessionMap;

//! This WorkSession completes the basic one, by adding :
//! - use of Controller, with norm selection...
//! - management of transfers (both ways) with auxiliary classes
//! TransferReader and TransferWriter
//! -> these transfers may work with a Context List : its items
//! are given by the user, according to the transfer to be
//! i.e. it is interpreted by the Actors
//! Each item is accessed by a Name
class DE_WorkSession : public Standard_Transient
{
public:

  Standard_EXPORT DE_WorkSession();

  Standard_EXPORT ~DE_WorkSession();

  //! Returns the current reference WS List, empty if not defined
  const XSControl_WorkSessionMap& ReferenceWS() const { return myReferenceWS; }

  //! Appends new refrence Work Session
  Standard_EXPORT void AppendReferenceWS(const TCollection_AsciiString& theName,
                                         const Handle(DE_WorkSession)& theWS);

  Handle(DE_DataModel) Model() { return myModel; }

  const Handle(DE_DataModel)& Model() const  { return myModel; }

  Standard_EXPORT const TCollection_AsciiString& Vendor() const;

  Standard_EXPORT const TCollection_AsciiString& Format() const;

  DEFINE_STANDARD_RTTIEXT(DE_WorkSession, Standard_Transient)

private:

  XSControl_WorkSessionMap myReferenceWS;
  Handle(DE_DataModel) myModel;

};

#endif // _XSControl_WorkSession_HeaderFile
