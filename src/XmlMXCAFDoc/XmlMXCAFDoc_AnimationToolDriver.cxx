// Created on: 2017-10-02
// Created by: Elena MOZOKHINA
// Copyright (c) 2016 OPEN CASCADE SAS
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
#include <XCAFDoc_AnimationTool.hxx>
#include <XmlMXCAFDoc_AnimationToolDriver.hxx>
#include <XmlObjMgt_Persistent.hxx>

IMPLEMENT_STANDARD_RTTIEXT(XmlMXCAFDoc_AnimationToolDriver,XmlMDF_ADriver)

//=======================================================================
//function :
//purpose  : 
//=======================================================================
XmlMXCAFDoc_AnimationToolDriver::XmlMXCAFDoc_AnimationToolDriver
  (const Handle(Message_Messenger)& theMsgDriver)
: XmlMDF_ADriver (theMsgDriver, "xcaf", "AnimationTool")
{
}

//=======================================================================
//function :
//purpose  : 
//=======================================================================
Handle(TDF_Attribute) XmlMXCAFDoc_AnimationToolDriver::NewEmpty() const
{
  return new XCAFDoc_AnimationTool();
}

//=======================================================================
//function :
//purpose  : 
//=======================================================================
Standard_Boolean XmlMXCAFDoc_AnimationToolDriver::Paste(const XmlObjMgt_Persistent& ,
                                                       const Handle(TDF_Attribute)& ,
                                                       XmlObjMgt_RRelocationTable& ) const
{
  return Standard_True;
}

//=======================================================================
//function :
//purpose  : 
//=======================================================================
void XmlMXCAFDoc_AnimationToolDriver::Paste(const Handle(TDF_Attribute)& ,
                                           XmlObjMgt_Persistent& ,
                                           XmlObjMgt_SRelocationTable& ) const
{
}
