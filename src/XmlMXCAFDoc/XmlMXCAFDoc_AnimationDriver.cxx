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
#include <TCollection_HAsciiString.hxx>
#include <TColStd_HArray1OfByte.hxx>
#include <TDF_Attribute.hxx>
#include <XCAFDoc_Animation.hxx>
#include <XmlMXCAFDoc_AnimationDriver.hxx>
#include <XmlObjMgt.hxx>
#include <XmlObjMgt_Persistent.hxx>
#include <LDOM_OSStream.hxx>

IMPLEMENT_STANDARD_RTTIEXT(XmlMXCAFDoc_AnimationDriver,XmlMDF_ADriver)
IMPLEMENT_DOMSTRING (AnimationName,     "name")
IMPLEMENT_DOMSTRING (ImageFirstIdx,    "image_first_idx")
IMPLEMENT_DOMSTRING (ImageLastIdx, "image_last_idx")
IMPLEMENT_DOMSTRING(AnimationFirstIdx, "animation_first_idx")
IMPLEMENT_DOMSTRING(AnimationLastIdx, "animation_last_idx")

//=======================================================================
//function : XmlMXCAFDoc_AnimationDriver
//purpose  : Constructor
//=======================================================================
XmlMXCAFDoc_AnimationDriver::XmlMXCAFDoc_AnimationDriver
  (const Handle(Message_Messenger)& theMsgDriver)
: XmlMDF_ADriver (theMsgDriver, "xcaf", "Animation")
{}

//=======================================================================
//function : NewEmpty
//purpose  : 
//=======================================================================
Handle(TDF_Attribute) XmlMXCAFDoc_AnimationDriver::NewEmpty() const
{
  return (new XCAFDoc_Animation());
}

//=======================================================================
//function : Paste
//purpose  : persistent -> transient (retrieve)
//=======================================================================
Standard_Boolean XmlMXCAFDoc_AnimationDriver::Paste
                                (const XmlObjMgt_Persistent&  theSource,
                                 const Handle(TDF_Attribute)& theTarget,
                                 XmlObjMgt_RRelocationTable&  ) const
{
  const XmlObjMgt_Element& anElement = theSource;
  Handle(XCAFDoc_Animation) aTarget = Handle(XCAFDoc_Animation)::DownCast(theTarget);
  if (aTarget.IsNull())
    return Standard_False;
  XmlObjMgt_DOMString aNameStr = anElement.getAttribute(::AnimationName());
  XmlObjMgt_DOMString anImageFirstIdxStr = anElement.getAttribute(::ImageFirstIdx());
  XmlObjMgt_DOMString anImageLastIdxStr = anElement.getAttribute(::ImageLastIdx());
  XmlObjMgt_DOMString anAnimFirstIdxStr = anElement.getAttribute(::AnimationFirstIdx());
  XmlObjMgt_DOMString anAnimLastIdxStr = anElement.getAttribute(::AnimationLastIdx());

  if (aNameStr == NULL || anImageFirstIdxStr == NULL ||  anImageLastIdxStr == NULL || 
      anAnimFirstIdxStr == NULL || anAnimLastIdxStr == NULL)
    return Standard_False;
  
  Standard_Integer anImageFirstIdx = 0;
  Standard_Integer anImageLastIdx = 0;
  if (!anImageFirstIdxStr.GetInteger(anImageFirstIdx) || !anImageLastIdxStr.GetInteger(anImageLastIdx))
    return Standard_False;
  Handle(TColStd_HArray1OfByte) anImage = new TColStd_HArray1OfByte(anImageFirstIdx, anImageLastIdx);
  
  Standard_Integer anAnimFirstIdx = 0;
  Standard_Integer anAnimLastIdx = 0;
  if (!anAnimFirstIdxStr.GetInteger(anAnimFirstIdx) || !anAnimLastIdxStr.GetInteger(anAnimLastIdx))
    return Standard_False;
  Handle(TColStd_HArray1OfByte) anAnim = new TColStd_HArray1OfByte(anAnimFirstIdx, anAnimLastIdx);
  
  //const Standard_Integer aMaxSize = anImageLastIdx - anImageFirstIdx + anAnimLastIdx - anAnimFirstIdx + 2;
  
  XmlObjMgt_DOMString aDataStr = XmlObjMgt::GetStringValue(theSource);
  Standard_SStream anSS(aDataStr.GetString());
  
  Standard_Byte aValue;
  for (Standard_Integer i = anImageFirstIdx; i <= anImageLastIdx; ++i)
  {
    anSS >> aValue;
    anImage->ChangeValue(i) = aValue;
  }

  for (Standard_Integer i = anAnimFirstIdx; i <= anAnimLastIdx; ++i)
  {
    anSS >> aValue;
    anAnim->ChangeValue(i) = aValue;
  }

  aTarget->Set(new TCollection_HAsciiString(aNameStr.GetString()), anImage, anImage);
  return Standard_True;
}

//=======================================================================
//function : Paste
//purpose  : transient -> persistent (store)
//=======================================================================
void XmlMXCAFDoc_AnimationDriver::Paste(const Handle(TDF_Attribute)& theSource,
  XmlObjMgt_Persistent&        theTarget,
  XmlObjMgt_SRelocationTable&) const
{
  Handle(XCAFDoc_Animation) anAnimation = Handle(XCAFDoc_Animation)::DownCast(theSource);
  if (anAnimation.IsNull())
    return;

  XmlObjMgt_DOMString aName = anAnimation->GetName()->String().ToCString();
  theTarget.Element().setAttribute(::AnimationName(), aName);
  
  Handle(TColStd_HArray1OfByte) anImage = anAnimation->GetImage();
  Handle(TColStd_HArray1OfByte) anAnim = anAnimation->GetAnimation();
  if (anImage.IsNull() || anAnim.IsNull())
  {
    return;
  }
  const Standard_Integer anImageFirstInd = anImage->Lower();
  const Standard_Integer anImageLastInd = anAnim->Upper();

  const Standard_Integer anAnimFirstInd = anAnim->Lower();
  const Standard_Integer anAnimLastInd = anAnim->Upper();

  const Standard_Integer aMaxSize = anImageLastInd - anImageFirstInd + anAnimLastInd - anAnimFirstInd + 2;

  LDOM_OSStream anOSS(aMaxSize);
  
  for (Standard_Integer i = anImageFirstInd; i <= anImageLastInd; ++i)
  {
    anOSS << std::hex << anImage->Value(i);
  }
  for (Standard_Integer i = anAnimFirstInd; i <= anAnimLastInd; ++i)
  {
    anOSS << std::hex << anAnim->Value(i);
  }
  Standard_Character* dump = (Standard_Character*)anOSS.str(); // copying! Don't forget to delete it.
  XmlObjMgt::SetStringValue(theTarget, dump, Standard_True);
  delete[] dump;
}
