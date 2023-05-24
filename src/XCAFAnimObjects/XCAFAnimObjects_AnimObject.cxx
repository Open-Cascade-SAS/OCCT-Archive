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

#include <BRep_Builder.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <Standard_Type.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TColStd_SequenceOfHAsciiString.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDataStd_UAttribute.hxx>
#include <TDF_Attribute.hxx>
#include <TDF_ChildIDIterator.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Label.hxx>
#include <TDF_LabelMap.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDF_RelocationTable.hxx>
#include <TDF_Tool.hxx>
#include <TDocStd_Document.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming_Tool.hxx>
#include <TopLoc_IndexedMapOfLocation.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_MapOfOrientedShape.hxx>
#include <XCAFDoc.hxx>
#include <XCAFDoc_GraphNode.hxx>
#include <XCAFDoc_Location.hxx>
#include <XCAFDoc_ShapeMapTool.hxx>

IMPLEMENT_DERIVED_ATTRIBUTE_WITH_TYPE(XCAFDoc_AnimationTool, TDataStd_GenericEmpty, "xcaf", "AnimationTool")

//=======================================================================
//function : GetID
//purpose  :
//=======================================================================
const Standard_GUID& XCAFDoc_AnimationTool::GetID()
{
  static Standard_GUID ShapeToolID("9E9914DD-154A-4E17-B89B-3E33CCF67BD0");
  return ShapeToolID;
}

//=======================================================================
//function : Set
//purpose  :
//=======================================================================
Handle(XCAFDoc_AnimationTool) XCAFDoc_AnimationTool::Set(const TDF_Label& theLabel)
{
  Handle(XCAFDoc_AnimationTool) anAnimTool;
  if (!theLabel.FindAttribute(XCAFDoc_AnimationTool::GetID(), anAnimTool)) {
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
//function : Dump
//purpose  :
//=======================================================================
Standard_OStream& XCAFDoc_AnimationTool::Dump(Standard_OStream& theOStream,
                                              const Standard_Boolean theDepth) const
{
  (void)theOStream;
  (void)theDepth;
}

//=======================================================================
//function : Dump
//purpose  :
//=======================================================================
Standard_OStream& XCAFDoc_AnimationTool::Dump(Standard_OStream& theDumpLog) const
{
  TDF_Attribute::Dump(theDumpLog);
  Dump(theDumpLog, Standard_False);
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
