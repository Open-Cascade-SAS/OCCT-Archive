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

#include <TCollection_HAsciiString.hxx>
#include <TDataStd_Integer.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDataXtd_Geometry.hxx>
#include <TDataXtd_Plane.hxx>
#include <TDF_Attribute.hxx>
#include <TDF_ChildIDIterator.hxx>
#include <XCAFDoc.hxx>
#include <XCAFDoc_Animation.hxx>
#include <XCAFDoc_AnimationTool.hxx>
#include <TDF_AttributeIterator.hxx>

IMPLEMENT_STANDARD_RTTIEXT(XCAFDoc_AnimationTool, TDF_Attribute)

//=======================================================================
//function : BaseLabel
//purpose  : 
//=======================================================================

TDF_Label XCAFDoc_AnimationTool::BaseLabel() const
{
  return Label();
}

//=======================================================================
//function : IsAnimation
//purpose  : 
//=======================================================================

Standard_Boolean XCAFDoc_AnimationTool::IsAnimation(const TDF_Label& theLabel) const
{
  if (theLabel.Father() != Label())
    return Standard_False;

  Handle(XCAFDoc_Animation) anAnimAttribute;
  if (!theLabel.FindAttribute(XCAFDoc_Animation::GetID(), anAnimAttribute))
    return Standard_False;


  return Standard_True;
}

//=======================================================================
//function : GetAnimation
//purpose  : 
//=======================================================================

Standard_Boolean XCAFDoc_AnimationTool::GetAnimation(const TDF_Label& theLabel,
                                                    Handle(TCollection_HAsciiString)& theName,
                                                    Handle(TColStd_HArray1OfByte)& theImage,
                                                    Handle(TColStd_HArray1OfByte)& theAnimation) const
{

  if (theLabel.Father() != Label())
    return Standard_False;

  Handle(XCAFDoc_Animation) animAttr;
  if (!theLabel.FindAttribute(XCAFDoc_Animation::GetID(), animAttr)) {
    return Standard_False;
  }
  theName = animAttr->GetName();
  theImage = animAttr->GetImage();
  theAnimation = animAttr->GetAnimation();
  return Standard_True;
}

//=======================================================================
//function : AddAnimation
//purpose  : 
//=======================================================================

TDF_Label XCAFDoc_AnimationTool::AddAnimation(const Handle(TCollection_HAsciiString)& theName,
                                              const Handle(TColStd_HArray1OfByte)& theImage,
                                              const Handle(TColStd_HArray1OfByte)& theAnimation )
{
  TDF_Label animL;
  TDF_TagSource aTag;
  animL = aTag.NewChild(Label());
  XCAFDoc_Animation::Set(animL, theName, theImage, theAnimation);
  TDataStd_Name::Set(animL, TCollection_AsciiString(theName->ToCString()));
  return animL;
}

//=======================================================================
//function : AddAnimation
//purpose  : 
//=======================================================================

TDF_Label XCAFDoc_AnimationTool::AddAnimation(const Handle(TCollection_HAsciiString)& theName,
                                              OSD_File& theImageFile, OSD_File& theAnimationFile)
{
  TDF_Label animL;
  TDF_TagSource aTag;
  animL = aTag.NewChild(Label());
  TDataStd_Name::Set(animL, TCollection_AsciiString(theName->ToCString()));
  XCAFDoc_Animation::Set(animL, theName, theImageFile, theAnimationFile);
  return animL;
}

//=======================================================================
//function : RemoveAnimation
//purpose  : 
//=======================================================================

Standard_Boolean XCAFDoc_AnimationTool::RemoveAnimation(const TDF_Label& theLabel) const
{
  Handle(XCAFDoc_Animation)anAnimAttr;
  if (!IsAnimation(theLabel) || !theLabel.FindAttribute(XCAFDoc_Animation::GetID(), anAnimAttr))
    return Standard_False;

  theLabel.ForgetAllAttributes(Standard_True);
  return Standard_True;
}

//=======================================================================
//function : GetAnimationLabels
//purpose  : 
//=======================================================================

void XCAFDoc_AnimationTool::GetAnimationLabels(TDF_LabelSequence& theLabels) const
{
  theLabels.Clear();
  TDF_ChildIterator aChildIterator(Label());
  for (; aChildIterator.More(); aChildIterator.Next()) {
    TDF_Label aLabel = aChildIterator.Value();
    if (IsAnimation(aLabel)) theLabels.Append(aLabel);
  }
}

//=======================================================================
//function : SetName
//purpose  : 
//=======================================================================

void XCAFDoc_AnimationTool::SetName(const TDF_Label& theLabel,
                                            const Handle(TCollection_HAsciiString)& theName) const
{
  if (theLabel.Father() != Label())
    return;

  Handle(XCAFDoc_Animation)anAnimAttr;
  if (!theLabel.FindAttribute(XCAFDoc_Animation::GetID(), anAnimAttr))
    return;
  anAnimAttr->Set(theName);
}

//=======================================================================
//function : GetID
//purpose  : 
//=======================================================================

const Standard_GUID& XCAFDoc_AnimationTool::GetID() 
{
  static Standard_GUID AnimationID("7261F539-43AD-4544-8419-AE63C6ED4A41");
  return AnimationID;
}

//=======================================================================
//function : Set
//purpose  : 
//=======================================================================

Handle(XCAFDoc_AnimationTool) XCAFDoc_AnimationTool::Set(const TDF_Label& L)
{
  Handle(XCAFDoc_AnimationTool) A;
  if (!L.FindAttribute(XCAFDoc_AnimationTool::GetID(), A)) {
    A = new XCAFDoc_AnimationTool();
    L.AddAttribute(A);
  }
  return A;
}

//=======================================================================
//function : ID
//purpose  : 
//=======================================================================

const Standard_GUID& XCAFDoc_AnimationTool::ID() const
{
  return GetID();
}

//=======================================================================
//function : Restore
//purpose  : 
//=======================================================================

void XCAFDoc_AnimationTool::Restore(const Handle(TDF_Attribute)& /*with*/)
{
}

//=======================================================================
//function : NewEmpty
//purpose  : 
//=======================================================================

Handle(TDF_Attribute) XCAFDoc_AnimationTool::NewEmpty() const
{
  return new XCAFDoc_AnimationTool;
}

//=======================================================================
//function : Paste
//purpose  : 
//=======================================================================

void XCAFDoc_AnimationTool::Paste(const Handle(TDF_Attribute)& /*into*/,
  const Handle(TDF_RelocationTable)& /*RT*/) const
{
}

//=======================================================================
//function : XCAFDoc_AnimationTool
//purpose  : 
//=======================================================================

XCAFDoc_AnimationTool::XCAFDoc_AnimationTool()
{
}

