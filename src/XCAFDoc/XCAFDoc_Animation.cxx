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

#include <XCAFDoc_Animation.hxx>

#include <TDF_RelocationTable.hxx>
#include <TDF_ChildIterator.hxx>
#include <XCAFDoc.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <TDataStd_Integer.hxx>
#include <TDataStd_IntegerArray.hxx>
#include <TDataStd_ExtStringArray.hxx>
#include <TDataStd_Real.hxx>
#include <TDataStd_RealArray.hxx>
#include <TNaming_Builder.hxx>
#include <TColStd_HArray1OfReal.hxx>
#include <TopoDS.hxx>
#include <XCAFDimTolObjects_DimensionObject.hxx>
#include <TNaming_Tool.hxx>
#include <TDataStd_Name.hxx>

IMPLEMENT_DERIVED_ATTRIBUTE(XCAFDoc_Animation, TDataStd_GenericEmpty)
enum ChildLab
{
  ChildLab_Begin = 1,
  ChildLab_Type = ChildLab_Begin,

  ChildLab_End
};

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
  static Standard_GUID DGTID("58ed092c-44de-11d8-8776-001083004c77");
  //static Standard_GUID ID("efd212e9-6dfd-11d4-b9c8-0060b0ee281b");
  return DGTID;
  //return ID;
}

//=======================================================================
//function : Set
//purpose  : 
//=======================================================================
Handle(XCAFDoc_Animation) XCAFDoc_Animation::Set(const TDF_Label& theLabel)
{
  Handle(XCAFDoc_Animation) A;
  if (!theLabel.FindAttribute(XCAFDoc_Animation::GetID(), A)) {
    A = new XCAFDoc_Animation();
    theLabel.AddAttribute(A);
  }
  return A;
}

//=======================================================================
//function : SetObject
//purpose  : 
//=======================================================================
void XCAFDoc_Animation::SetObject(const Handle(XCAFAnimObjects_AnimObject)& theObject)
{
  Backup();

}

//=======================================================================
//function : GetObject
//purpose  : 
//=======================================================================
Handle(XCAFAnimObjects_AnimObject) XCAFDoc_Animation::GetObject()  const
{
  Handle(XCAFAnimObjects_AnimObject) anObj = new XCAFAnimObjects_AnimObject();

  return anObj;
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
//function : DumpJson
//purpose  :
//=======================================================================
void XCAFDoc_Animation::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{

}
