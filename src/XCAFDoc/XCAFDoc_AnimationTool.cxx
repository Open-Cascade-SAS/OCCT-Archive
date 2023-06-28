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

#include <XCAFDoc_AnimationTool.hxx>

#include <Standard_GUID.hxx>
#include <TDataStd_AsciiString.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_Real.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDataStd_UAttribute.hxx>
#include <TDF_ChildIterator.hxx>
#include <XCAFDoc.hxx>
#include <XCAFDoc_Animation.hxx>
#include <XCAFDoc_GraphNode.hxx>

IMPLEMENT_DERIVED_ATTRIBUTE_WITH_TYPE(XCAFDoc_AnimationTool, TDataStd_GenericEmpty, "xcaf", "AnimationTool")

namespace
{
  //=======================================================================
  //function : GetGlobalFPSGUID
  //purpose  :
  //=======================================================================
  const Standard_GUID& GetGlobalFPSGUID()
  {
    static Standard_GUID aGlobalFPSID("C7E7AF70-2FB3-40FD-BD38-CC79D9343D7A");
    return aGlobalFPSID;
  }

  //=======================================================================
  //function : GetStartTimeCodeGUID
  //purpose  :
  //=======================================================================
  const Standard_GUID& GetStartTimeCodeGUID()
  {
    static Standard_GUID aStartTimeCodeGUID("379BC1C5-E9DA-4B57-9938-B2612158722A");
    return aStartTimeCodeGUID;
  }

  //=======================================================================
  //function : GetEndTimeCodeGUID
  //purpose  :
  //=======================================================================
  const Standard_GUID& GetEndTimeCodeGUID()
  {
    static Standard_GUID aEndTimeCodeGUID("EF5305A3-961D-48AE-9A78-AC744A110A26");
    return aEndTimeCodeGUID;
  }

  //=======================================================================
  //function : GetFileLengthUnitGUID
  //purpose  :
  //=======================================================================
  const Standard_GUID& GetFileLengthUnitGUID()
  {
    static Standard_GUID aFileLengthUnitGUID("492f5372-9a28-4611-a663-d8394f98df00");
    return aFileLengthUnitGUID;
  }

  //=======================================================================
  //function : GetFileLengthUnitGUID
  //purpose  :
  //=======================================================================
  const Standard_GUID& GetUpAxisGUID()
  {
    static Standard_GUID anUpAxisGUID("05d55dd9-7175-44a7-97aa-43909ad6f9c7");
    return anUpAxisGUID;
  }
}

//=======================================================================
//function : GetID
//purpose  :
//=======================================================================
const Standard_GUID& XCAFDoc_AnimationTool::GetID()
{
  static Standard_GUID anAnimationToolID("9E9914DD-154A-4E17-B89B-3E33CCF67BD0");
  return anAnimationToolID;
}

//=======================================================================
//function : Set
//purpose  :
//=======================================================================
Handle(XCAFDoc_AnimationTool) XCAFDoc_AnimationTool::Set(const TDF_Label& theLabel)
{
  Handle(XCAFDoc_AnimationTool) anAnimTool;
  if (!theLabel.FindAttribute(XCAFDoc_AnimationTool::GetID(), anAnimTool))
  {
    anAnimTool = new XCAFDoc_AnimationTool();
    theLabel.AddAttribute(anAnimTool);
  }
  return anAnimTool;
}

//=======================================================================
//function : Constructor
//purpose  :
//=======================================================================
XCAFDoc_AnimationTool::XCAFDoc_AnimationTool()
{}

//=======================================================================
//function : ID
//purpose  :
//=======================================================================
const Standard_GUID& XCAFDoc_AnimationTool::ID() const
{
  return GetID();
}

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
bool XCAFDoc_AnimationTool::IsAnimation(const TDF_Label& theLabel) const
{
  Handle(XCAFDoc_Animation) anAnimAtr;
  if (theLabel.FindAttribute(XCAFDoc_Animation::GetID(), anAnimAtr))
  {
    return true;
  }
  return false;
}

//=======================================================================
//function : SetAnimation
//purpose  :
//=======================================================================
void XCAFDoc_AnimationTool::SetAnimation(const TDF_Label& theShLabel,
                                         const TDF_Label& theAnimLabel) const
{
  // set reference
  Handle(TDataStd_TreeNode) aRefNode, aMainNode;
  aMainNode = TDataStd_TreeNode::Set(theAnimLabel, XCAFDoc::AnimRefShapeGUID());
  aRefNode = TDataStd_TreeNode::Set(theShLabel, XCAFDoc::AnimRefShapeGUID());
  aRefNode->Remove();
  aMainNode->Prepend(aRefNode);
}

//=======================================================================
//function : GetGlobalFPS
//purpose  :
//=======================================================================
bool XCAFDoc_AnimationTool::GetGlobalFPS(double& theFPS) const
{
  Handle(TDataStd_Real) aFPSAttr;
  if (BaseLabel().FindAttribute(GetGlobalFPSGUID(), aFPSAttr))
  {
    theFPS = aFPSAttr->Get();
    return true;
  }
  return false;
}

//=======================================================================
//function : SetGlobalFPS
//purpose  :
//=======================================================================
void XCAFDoc_AnimationTool::SetGlobalFPS(const double theFPS) const
{
  Handle(TDataStd_Real) aFPSAttr;
  if (BaseLabel().FindAttribute(GetGlobalFPSGUID(), aFPSAttr))
  {
    aFPSAttr->Set(theFPS);
    return;
  }
  TDataStd_Real::Set(BaseLabel(), GetGlobalFPSGUID(), theFPS);
}

//=======================================================================
//function : GetStartTimeCode
//purpose  :
//=======================================================================
bool XCAFDoc_AnimationTool::GetStartTimeCode(double& theCode) const
{
  Handle(TDataStd_Real) aCodeAttr;
  if (BaseLabel().FindAttribute(GetStartTimeCodeGUID(), aCodeAttr))
  {
    theCode = aCodeAttr->Get();
    return true;
  }
  return false;
}

//=======================================================================
//function : SetStartTimeCode
//purpose  :
//=======================================================================
void XCAFDoc_AnimationTool::SetStartTimeCode(const double theCode) const
{
  Handle(TDataStd_Real) aCodeAttr;
  if (BaseLabel().FindAttribute(GetStartTimeCodeGUID(), aCodeAttr))
  {
    aCodeAttr->Set(theCode);
    return;
  }
  TDataStd_Real::Set(BaseLabel(), GetStartTimeCodeGUID(), theCode);
}

//=======================================================================
//function : GetEndTimeCode
//purpose  :
//=======================================================================
bool XCAFDoc_AnimationTool::GetEndTimeCode(double& theCode) const
{
  Handle(TDataStd_Real) aCodeAttr;
  if (BaseLabel().FindAttribute(GetEndTimeCodeGUID(), aCodeAttr))
  {
    theCode = aCodeAttr->Get();
    return true;
  }
  return false;
}

//=======================================================================
//function : SetEndTimeCode
//purpose  :
//=======================================================================
void XCAFDoc_AnimationTool::SetEndTimeCode(const double theCode) const
{
  Handle(TDataStd_Real) aCodeAttr;
  if (BaseLabel().FindAttribute(GetEndTimeCodeGUID(), aCodeAttr))
  {
    aCodeAttr->Set(theCode);
    return;
  }
  TDataStd_Real::Set(BaseLabel(), GetEndTimeCodeGUID(), theCode);
}

//=======================================================================
//function : GetFileLengthUnit
//purpose  :
//=======================================================================
bool XCAFDoc_AnimationTool::GetFileLengthUnit(double& theLengthUnit) const
{
  Handle(TDataStd_Real) aLengthUnitAttr;
  if (BaseLabel().FindAttribute(GetFileLengthUnitGUID(), aLengthUnitAttr))
  {
    theLengthUnit = aLengthUnitAttr->Get();
    return true;
  }
  return false;
}

//=======================================================================
//function : SetFileLengthUnit
//purpose  :
//=======================================================================
void XCAFDoc_AnimationTool::SetFileLengthUnit(const double theLengthUnit) const
{
  Handle(TDataStd_Real) aLengthUnitAttr;
  if (BaseLabel().FindAttribute(GetFileLengthUnitGUID(), aLengthUnitAttr))
  {
    aLengthUnitAttr->Set(theLengthUnit);
    return;
  }
  TDataStd_Real::Set(BaseLabel(), GetFileLengthUnitGUID(), theLengthUnit);
}

//=======================================================================
//function : GetUpAxis
//purpose  :
//=======================================================================
bool XCAFDoc_AnimationTool::GetUpAxis(TCollection_AsciiString& theAxis) const
{
  Handle(TDataStd_AsciiString) anAxisAttr;
  if (BaseLabel().FindAttribute(GetUpAxisGUID(), anAxisAttr))
  {
    theAxis = anAxisAttr->Get();
    return true;
  }
  return false;
}

//=======================================================================
//function : SetUpAxis
//purpose  :
//=======================================================================
void XCAFDoc_AnimationTool::SetUpAxis(const TCollection_AsciiString theAxis) const
{
  Handle(TDataStd_AsciiString) anAxisAttr;
  if (BaseLabel().FindAttribute(GetUpAxisGUID(), anAxisAttr))
  {
    anAxisAttr->Set(theAxis);
    return;
  }
  TDataStd_AsciiString::Set(BaseLabel(), GetUpAxisGUID(), theAxis);
}

//=======================================================================
//function : GetAnimationLabels
//purpose  :
//=======================================================================
void XCAFDoc_AnimationTool::GetAnimationLabels(TDF_LabelSequence& theLabels) const
{
  theLabels.Clear();
  for (TDF_ChildIterator aChildIterator(Label());
       aChildIterator.More(); aChildIterator.Next())
  {
    TDF_Label aL = aChildIterator.Value();
    if (IsAnimation(aL))
    {
      theLabels.Append(aL);
    }
  }
}

//=======================================================================
//function : GetRefAnimationLabels
//purpose  :
//=======================================================================
bool XCAFDoc_AnimationTool::GetRefAnimationLabel(const TDF_Label& theShLabel,
                                                 TDF_Label& theAnimLabel) const
{
  Handle(TDataStd_TreeNode) aNode;
  if (!theShLabel.FindAttribute(XCAFDoc::AnimRefShapeGUID(), aNode) ||
      !aNode->HasFather())
  {
    return false;
  }
  theAnimLabel = aNode->Father()->Label();
  return true;
}

//=======================================================================
//function : GetRefShapeLabel
//purpose  :
//=======================================================================
bool XCAFDoc_AnimationTool::GetRefShapeLabel(const TDF_Label& theAnimLabel,
                                             TDF_Label& theShLabel) const
{
  Handle(TDataStd_TreeNode) aNode;
  if (!theAnimLabel.FindAttribute(XCAFDoc::AnimRefShapeGUID(), aNode) ||
      !aNode->Label().IsNull())
  {
    return false;
  }
  theShLabel = aNode->Label();
  return true;
}

//=======================================================================
//function : AddAnimation
//purpose  :
//=======================================================================
TDF_Label XCAFDoc_AnimationTool::AddAnimation() const
{
  TDF_Label anAnimL;
  TDF_TagSource aTag;
  anAnimL = aTag.NewChild(Label());
  Handle(XCAFDoc_Animation) aTol = XCAFDoc_Animation::Set(anAnimL);
  TCollection_AsciiString aStr = "Animation";
  TDataStd_Name::Set(anAnimL, aStr);
  return anAnimL;
}

//=======================================================================
//function : IsLocked
//purpose  :
//=======================================================================
bool XCAFDoc_AnimationTool::IsLocked(const TDF_Label& theAnimLabel) const
{
  Handle(TDataStd_UAttribute) anAttr;
  return theAnimLabel.FindAttribute(XCAFDoc::LockGUID(), anAttr);
}

//=======================================================================
//function : Unlock
//purpose  :
//=======================================================================
void XCAFDoc_AnimationTool::Lock(const TDF_Label& theAnimLabel) const
{
  TDataStd_UAttribute::Set(theAnimLabel, XCAFDoc::LockGUID());
}

//=======================================================================
//function : Unlock
//purpose  :
//=======================================================================
void XCAFDoc_AnimationTool::Unlock(const TDF_Label& theAnimLabel) const
{
  theAnimLabel.ForgetAttribute(XCAFDoc::LockGUID());
}

//=======================================================================
//function : Dump
//purpose  :
//=======================================================================
Standard_OStream& XCAFDoc_AnimationTool::Dump(Standard_OStream& theOStream,
                                              const bool theDepth) const
{
  (void)theDepth;
  return theOStream;
}

//=======================================================================
//function : Dump
//purpose  :
//=======================================================================
Standard_OStream& XCAFDoc_AnimationTool::Dump(Standard_OStream& theDumpLog) const
{
  TDF_Attribute::Dump(theDumpLog);
  Dump(theDumpLog, false);
  return theDumpLog;
}

//=======================================================================
//function : DumpJson
//purpose  :
//=======================================================================
void XCAFDoc_AnimationTool::DumpJson(Standard_OStream& theOStream,
                                     Standard_Integer theDepth) const
{
  (void)theOStream;
  (void)theDepth;
}
