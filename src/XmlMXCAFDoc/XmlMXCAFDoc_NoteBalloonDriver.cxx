// Created on: 2017-08-10
// Created by: Sergey NIKONOV
// Copyright (c) 2008-2017 OPEN CASCADE SAS
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

#include <Message_Messenger.hxx>
#include <Standard_Type.hxx>
#include <TDF_Attribute.hxx>
#include <XCAFDoc_NoteBalloon.hxx>
#include <XmlMXCAFDoc_NoteBalloonDriver.hxx>
#include <XmlObjMgt_Persistent.hxx>

IMPLEMENT_STANDARD_RTTIEXT(XmlMXCAFDoc_NoteBalloonDriver, XmlMXCAFDoc_NoteDriver)
IMPLEMENT_DOMSTRING(Comment, "comment")

//=======================================================================
//function :
//purpose  : 
//=======================================================================
XmlMXCAFDoc_NoteBalloonDriver::XmlMXCAFDoc_NoteBalloonDriver(const Handle(Message_Messenger)& theMsgDriver)
  : XmlMXCAFDoc_NoteDriver(theMsgDriver, STANDARD_TYPE(XCAFDoc_NoteBalloon)->Name())
{
}

//=======================================================================
//function :
//purpose  : 
//=======================================================================
Handle(TDF_Attribute) XmlMXCAFDoc_NoteBalloonDriver::NewEmpty() const
{
  return new XCAFDoc_NoteBalloon();
}

//=======================================================================
//function :
//purpose  : 
//=======================================================================
Standard_Boolean XmlMXCAFDoc_NoteBalloonDriver::Paste(const XmlObjMgt_Persistent&  theSource,
                                                      const Handle(TDF_Attribute)& theTarget,
                                                      XmlObjMgt_RRelocationTable&  theRelocTable) const
{
  XmlMXCAFDoc_NoteDriver::Paste(theSource, theTarget, theRelocTable);

  const XmlObjMgt_Element& anElement = theSource;

  XmlObjMgt_DOMString aComment = anElement.getAttribute(::Comment());
  if (aComment == NULL)
    return Standard_False;

  Handle(XCAFDoc_NoteBalloon) aNote = Handle(XCAFDoc_NoteBalloon)::DownCast(theTarget);
  if (aNote.IsNull())
    return Standard_False;

  aNote->Set(aComment.GetString());

  return Standard_True;
}

//=======================================================================
//function :
//purpose  : 
//=======================================================================
void XmlMXCAFDoc_NoteBalloonDriver::Paste(const Handle(TDF_Attribute)& theSource,
                                          XmlObjMgt_Persistent&        theTarget,
                                          XmlObjMgt_SRelocationTable&  theRelocTable) const
{
  XmlMXCAFDoc_NoteDriver::Paste(theSource, theTarget, theRelocTable);

  Handle(XCAFDoc_NoteBalloon) aNote = Handle(XCAFDoc_NoteBalloon)::DownCast(theSource);

  XmlObjMgt_DOMString aComment(TCollection_AsciiString(aNote->TimeStamp()).ToCString());

  theTarget.Element().setAttribute(::Comment(), aComment);
}

//=======================================================================
//function :
//purpose  : 
//=======================================================================
XmlMXCAFDoc_NoteBalloonDriver::XmlMXCAFDoc_NoteBalloonDriver(const Handle(Message_Messenger)& theMsgDriver,
                                                             Standard_CString                 theName)
  : XmlMXCAFDoc_NoteDriver(theMsgDriver, theName)
{

}
