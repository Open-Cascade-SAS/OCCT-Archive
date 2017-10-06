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


#include <BinMXCAFDoc_AnimationToolDriver.hxx>
#include <BinObjMgt_Persistent.hxx>
#include <Message_Messenger.hxx>
#include <Standard_Type.hxx>
#include <TDF_Attribute.hxx>
#include <XCAFDoc_AnimationTool.hxx>

IMPLEMENT_STANDARD_RTTIEXT(BinMXCAFDoc_AnimationToolDriver, BinMDF_ADriver)

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
BinMXCAFDoc_AnimationToolDriver::BinMXCAFDoc_AnimationToolDriver
  (const Handle(Message_Messenger)& theMsgDriver)
  : BinMDF_ADriver(theMsgDriver, STANDARD_TYPE(XCAFDoc_AnimationTool)->Name())
{
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
Handle(TDF_Attribute) BinMXCAFDoc_AnimationToolDriver::NewEmpty() const
{
  return new XCAFDoc_AnimationTool();
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
Standard_Boolean BinMXCAFDoc_AnimationToolDriver::Paste
  (const BinObjMgt_Persistent& /*theSource*/,
  const Handle(TDF_Attribute)& /*theTarget*/,
  BinObjMgt_RRelocationTable& /*theRelocTable*/) const
{
  return Standard_True;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void BinMXCAFDoc_AnimationToolDriver::Paste
  (const Handle(TDF_Attribute)& /*theSource*/,
   BinObjMgt_Persistent& /*theTarget*/,
   BinObjMgt_SRelocationTable& /*theRelocTable*/) const {
}
