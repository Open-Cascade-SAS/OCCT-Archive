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


#include <BinMXCAFDoc_AnimationDriver.hxx>
#include <BinObjMgt_Persistent.hxx>
#include <Message_Messenger.hxx>
#include <Standard_Type.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TDF_Attribute.hxx>
#include <XCAFDoc_Animation.hxx>
# include <TColStd_HArray1OfByte.hxx>

IMPLEMENT_STANDARD_RTTIEXT(BinMXCAFDoc_AnimationDriver,BinMDF_ADriver)

static Standard_Boolean getByteArray(const BinObjMgt_Persistent& theSource, Handle(TColStd_HArray1OfByte)& theArray)
{
  Standard_Integer aFirstInd, aLastInd;
  if (!(theSource >> aFirstInd >> aLastInd))
    return Standard_False;
  if (aLastInd < aFirstInd)
    return Standard_False;

  theArray.reset(new TColStd_HArray1OfByte(aFirstInd, aLastInd - aFirstInd + 1));
  theSource.GetByteArray(&theArray->ChangeFirst(), aLastInd - aFirstInd + 1);
  return Standard_True;
}

static void putByteArray(BinObjMgt_Persistent& theTarget, const Handle(TColStd_HArray1OfByte)& theArray)
{
  if (theArray.IsNull())
    return;
  const Standard_Integer aFirstInd = theArray->Lower();
  const Standard_Integer aLastInd = theArray->Upper();
  if (aLastInd < aFirstInd)
    return;
  
  theTarget << aFirstInd << aLastInd;
  theTarget.PutByteArray(&theArray->ChangeFirst(), aLastInd - aFirstInd + 1);
}

//=======================================================================
//function : Constructor
//purpose  : 
//=======================================================================
BinMXCAFDoc_AnimationDriver::BinMXCAFDoc_AnimationDriver
  (const Handle(Message_Messenger)& theMsgDriver)
: BinMDF_ADriver(theMsgDriver, STANDARD_TYPE(XCAFDoc_Animation)->Name())
{
}

//=======================================================================
//function : NewEmpty
//purpose  : 
//=======================================================================
Handle(TDF_Attribute) BinMXCAFDoc_AnimationDriver::NewEmpty() const
{
  return new XCAFDoc_Animation();
}


//=======================================================================
//function : Paste
//purpose  : 
//=======================================================================
Standard_Boolean BinMXCAFDoc_AnimationDriver::Paste(const BinObjMgt_Persistent& theSource,
                                                 const Handle(TDF_Attribute)& theTarget,
                                                 BinObjMgt_RRelocationTable& /*theRelocTable*/) const 
{
  Handle(XCAFDoc_Animation) anAtt = Handle(XCAFDoc_Animation)::DownCast(theTarget);
  Standard_Real aDensity;
  TCollection_AsciiString aName;
  
  if (!(theSource >> aName))
    return Standard_False;
  Handle(TColStd_HArray1OfByte) anImage, anAnimation;
  if (!getByteArray(theSource, anImage) || anImage.IsNull())
    return Standard_False;
  if (!getByteArray(theSource, anAnimation) || anAnimation.IsNull())
    return Standard_False;
  anAtt->Set(new TCollection_HAsciiString(aName), anImage, anAnimation);
  return Standard_True;
}

//=======================================================================
//function : Paste
//purpose  : 
//=======================================================================
void BinMXCAFDoc_AnimationDriver::Paste(const Handle(TDF_Attribute)& theSource,
                                     BinObjMgt_Persistent& theTarget,
                                     BinObjMgt_SRelocationTable& /*theRelocTable*/) const
{
  Handle(XCAFDoc_Animation) anAtt = Handle(XCAFDoc_Animation)::DownCast(theSource);
  Handle(TCollection_HAsciiString) aName = anAtt->GetName();
  if (!aName.IsNull())
    theTarget << aName->String();
  
  putByteArray(theTarget, anAtt->GetImage());
  putByteArray(theTarget, anAtt->GetAnimation());
}
