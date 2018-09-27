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


#include <Standard_GUID.hxx>
#include <Standard_Type.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TDF_Attribute.hxx>
#include <TDF_Label.hxx>
#include <TDF_RelocationTable.hxx>
#include <XCAFDoc_Animation.hxx>

#include <TColStd_HArray1OfByte.hxx>

IMPLEMENT_STANDARD_RTTIEXT(XCAFDoc_Animation, TDF_Attribute)

//=======================================================================
//function : XCAFDoc_Animation
//purpose  : 
//=======================================================================
XCAFDoc_Animation::XCAFDoc_Animation()
{
}


//=======================================================================
//function : GetID
//purpose  : 
//=======================================================================

const Standard_GUID& XCAFDoc_Animation::GetID()
{
  static Standard_GUID MatID("431A3BB7-7113-45C4-8653-AED7CC0012E1");
  return MatID;
}

//=======================================================================
//function : Set
//purpose  : 
//=======================================================================

Handle(XCAFDoc_Animation) XCAFDoc_Animation::Set(const TDF_Label& theLabel,
                                                 const Handle(TCollection_HAsciiString)& theName,
                                                 const Handle(TColStd_HArray1OfByte)& theImage,
                                                 const Handle(TColStd_HArray1OfByte)& theAnimation)
{
  Handle(XCAFDoc_Animation) A;
  if (!theLabel.FindAttribute(XCAFDoc_Animation::GetID(), A))
  {
    A = new XCAFDoc_Animation();
    theLabel.AddAttribute(A);
  }
  A->Set(theName, theImage, theAnimation);
  return A;
}
//=======================================================================
//function : Set
//purpose  : 
//=======================================================================

Handle(XCAFDoc_Animation) XCAFDoc_Animation::Set(const TDF_Label& theLabel,
                  const Handle(TCollection_HAsciiString)& theName,
                  OSD_File& theImageFile,  OSD_File& theAnimationFile)
{
  Handle(XCAFDoc_Animation) anAnimation;
  if (!theLabel.FindAttribute(XCAFDoc_Animation::GetID(), anAnimation))
  {
    anAnimation = new XCAFDoc_Animation();
    if (anAnimation->Set(theName, theImageFile, theAnimationFile))
      theLabel.AddAttribute(anAnimation);
    else
      anAnimation.Nullify();
  }
  else
    anAnimation->Set(theName, theImageFile, theAnimationFile);
  return anAnimation;
}
//=======================================================================
//function : Set
//purpose  : 
//=======================================================================
Standard_Boolean XCAFDoc_Animation::Set(const Handle(TCollection_HAsciiString)& theName, OSD_File& theImageFile, OSD_File& theAnimationFile)
{
  if (!theImageFile.IsOpen() || !theImageFile.IsReadable() || !theAnimationFile.IsOpen() || !theAnimationFile.IsReadable())
    return Standard_False;
  if (theImageFile.Size() > (Standard_Size)IntegerLast() || theAnimationFile.Size() > (Standard_Size)IntegerLast())
    return Standard_False;
  myImage.reset(new TColStd_HArray1OfByte(1, static_cast<Standard_Integer>(theImageFile.Size())));
  Standard_Integer nbReadBytes = 0;
  theImageFile.Read((Standard_Address)&myImage->First(), myImage->Length(), nbReadBytes);
  if (nbReadBytes < myImage->Length())
    return Standard_False;
  myAnimation.reset(new TColStd_HArray1OfByte(1, static_cast<Standard_Integer>(theAnimationFile.Size())));
  nbReadBytes = 0;
  theAnimationFile.Read((Standard_Address)&myAnimation->First(), myAnimation->Length(), nbReadBytes);
  if (nbReadBytes < myAnimation->Length())
    return Standard_False;
  myName = theName;
  return Standard_True;
}

//=======================================================================
//function : Set
//purpose  : 
//=======================================================================

void XCAFDoc_Animation::Set(const Handle(TCollection_HAsciiString)& theName, 
                            const Handle(TColStd_HArray1OfByte)& theImage,
                            const Handle(TColStd_HArray1OfByte)& theAnimation)
{
  myName = theName;
  myImage = theImage;
  myAnimation = theAnimation;
}

//=======================================================================
//function : Set
//purpose  : 
//=======================================================================

void XCAFDoc_Animation::Set(const Handle(TCollection_HAsciiString)& theName)
{
  myName = theName;
}
//=======================================================================
//function : GetName
//purpose  : 
//=======================================================================

Handle(TCollection_HAsciiString) XCAFDoc_Animation::GetName() const
{
  return myName;
}


//=======================================================================
//function : GetImage
//purpose  : 
//=======================================================================

Handle(TColStd_HArray1OfByte) XCAFDoc_Animation::GetImage() const
{
  return myImage;
}


//=======================================================================
//function : GetAnimation
//purpose  : 
//=======================================================================

Handle(TColStd_HArray1OfByte) XCAFDoc_Animation::GetAnimation() const
{
  return myAnimation;
}

//=======================================================================
//function : ID
//purpose  : 
//=======================================================================

const Standard_GUID& XCAFDoc_Animation::ID() const
{
  return GetID();
}


//=======================================================================
//function : Restore
//purpose  : 
//=======================================================================

void XCAFDoc_Animation::Restore(const Handle(TDF_Attribute)& With)
{
  myName = Handle(XCAFDoc_Animation)::DownCast(With)->GetName();
  myImage = Handle(XCAFDoc_Animation)::DownCast(With)->GetImage();
  myAnimation = Handle(XCAFDoc_Animation)::DownCast(With)->GetAnimation();
}


//=======================================================================
//function : NewEmpty
//purpose  : 
//=======================================================================

Handle(TDF_Attribute) XCAFDoc_Animation::NewEmpty() const
{
  return new XCAFDoc_Animation();
}


//=======================================================================
//function : Paste
//purpose  : 
//=======================================================================

void XCAFDoc_Animation::Paste(const Handle(TDF_Attribute)& Into,
  const Handle(TDF_RelocationTable)& /*RT*/) const
{
  Handle(XCAFDoc_Animation)::DownCast(Into)->Set(myName, myImage, myAnimation);
}

