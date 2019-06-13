// Copyright (c) 2017-2018 OPEN CASCADE SAS
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

#include <XCAFDoc_Note.hxx>

#include <gp_Pln.hxx>
#include <Standard_GUID.hxx>
#include <TDataXtd_Geometry.hxx>
#include <TDataXtd_Plane.hxx>
#include <TDataXtd_Point.hxx>
#include <TDF_AttributeIterator.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Label.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming_NamedShape.hxx>
#include <TNaming_Tool.hxx>
#include <XCAFDoc.hxx>
#include <XCAFDoc_GraphNode.hxx>

IMPLEMENT_STANDARD_RTTIEXT(XCAFDoc_Note, TDF_Attribute)

// =======================================================================
// function : IsMine
// purpose  :
// =======================================================================
Standard_Boolean
XCAFDoc_Note::IsMine(const TDF_Label& theLabel)
{
  return !theLabel.IsNull() && !Get(theLabel).IsNull();
}

// =======================================================================
// function : GetID
// purpose  :
// =======================================================================
const Standard_GUID&
XCAFDoc_Note::GetID()
{
  static Standard_GUID s_ID("A379384E-8412-4e34-BC26-097D503E2622");
  return s_ID;
}

// =======================================================================
// function : XCAFDoc_Note
// purpose  :
// =======================================================================
XCAFDoc_Note::XCAFDoc_Note()
{
}

// =======================================================================
// function : Get
// purpose  :
// =======================================================================
Handle(XCAFDoc_Note)
XCAFDoc_Note::Get(const TDF_Label& theLabel)
{
  Handle(XCAFDoc_Note) aNote;
  theLabel.FindAttribute(XCAFDoc_Note::GetID(), aNote);
  return aNote;
}

// =======================================================================
// function : Set
// purpose  :
// =======================================================================
Handle(XCAFDoc_Note) 
XCAFDoc_Note::Set(const TDF_Label&                  theLabel,
                  const TCollection_ExtendedString& theUserName,
                  const TCollection_ExtendedString& theTimeStamp)
{
  Handle(XCAFDoc_Note) aNote;
  if (!theLabel.IsNull() && !theLabel.FindAttribute(XCAFDoc_Note::GetID(), aNote))
  {
    aNote = new XCAFDoc_Note();
    aNote->XCAFDoc_Note::Set(theUserName, theTimeStamp);
    theLabel.AddAttribute(aNote);
  }
  return aNote;
}

// =======================================================================
// function : Set
// purpose  :
// =======================================================================
void
XCAFDoc_Note::Set(const TCollection_ExtendedString& theUserName,
                  const TCollection_ExtendedString& theTimeStamp)
{
  Backup();

  myUserName = theUserName;
  myTimeStamp = theTimeStamp;
}

// =======================================================================
// function : IsOrphan
// purpose  :
// =======================================================================
Standard_Boolean XCAFDoc_Note::IsOrphan() const
{
  Handle(XCAFDoc_GraphNode) aFather;
  return !Label().FindAttribute(XCAFDoc::NoteRefGUID(), aFather) ||
         (aFather->NbChildren() == 0);
}

// =======================================================================
// function : GetObject
// purpose  :
// =======================================================================
Handle(XCAFNoteObjects_NoteObject) XCAFDoc_Note::GetObject() const
{
  Handle(XCAFNoteObjects_NoteObject) anObj = new XCAFNoteObjects_NoteObject();

  Handle(TDataXtd_Point) aPnt;
  if (Label().FindChild(ChildLab_Pnt).FindAttribute(TDataXtd_Point::GetID(), aPnt))
  {
    gp_Pnt aP;
    if (TDataXtd_Geometry::Point(aPnt->Label(), aP))
    {
      anObj->SetPoint(aP);
    }
  }

  Handle(TDataXtd_Plane) aPln;
  if (Label().FindChild(ChildLab_Plane).FindAttribute(TDataXtd_Plane::GetID(), aPln))
  {
    gp_Pln aP;
    if (TDataXtd_Geometry::Plane(aPln->Label(), aP))
    {
      anObj->SetPlane(aP.Position().Ax2());
    }
  }

  Handle(TDataXtd_Point) aPntText;
  if (Label().FindChild(ChildLab_PntText).FindAttribute(TDataXtd_Point::GetID(), aPntText))
  {
    gp_Pnt aP;
    if (TDataXtd_Geometry::Point(aPntText->Label(), aP))
    {
      anObj->SetPointText(aP);
    }
  }

  Handle(TNaming_NamedShape) aNS;
  TDF_Label aLPres = Label().FindChild(ChildLab_Presentation);
  if (aLPres.FindAttribute(TNaming_NamedShape::GetID(), aNS))
  {
    TopoDS_Shape aPresentation = TNaming_Tool::GetShape(aNS);
    if (!aPresentation.IsNull())
    {
      anObj->SetPresentation(aPresentation);
    }
  }

  return anObj;
}

// =======================================================================
// function : SetObject
// purpose  :
// =======================================================================
void XCAFDoc_Note::SetObject (const Handle(XCAFNoteObjects_NoteObject)& theObject)
{
  Backup();

  TDF_Label aChLabel;
  aChLabel = Label().FindChild(ChildLab_PntText,      Standard_False); if (!aChLabel.IsNull()) aChLabel.ForgetAllAttributes();
  aChLabel = Label().FindChild(ChildLab_Plane,        Standard_False); if (!aChLabel.IsNull()) aChLabel.ForgetAllAttributes();
  aChLabel = Label().FindChild(ChildLab_Pnt,          Standard_False); if (!aChLabel.IsNull()) aChLabel.ForgetAllAttributes();
  aChLabel = Label().FindChild(ChildLab_Presentation, Standard_False); if (!aChLabel.IsNull()) aChLabel.ForgetAllAttributes();

  if (theObject->HasPoint())
  {
    gp_Pnt aPnt1 = theObject->GetPoint();
    TDataXtd_Point::Set (Label().FindChild (ChildLab_Pnt), aPnt1);
  }

  if (theObject->HasPlane())
  {
    gp_Ax2 anAx = theObject->GetPlane();

    gp_Pln aP (anAx);
    TDataXtd_Plane::Set (Label().FindChild (ChildLab_Plane), aP);
  }

  if (theObject->HasPointText())
  {
    gp_Pnt aPntText = theObject->GetPointText();
    TDataXtd_Point::Set (Label().FindChild (ChildLab_PntText), aPntText);
  }

  TopoDS_Shape aPresentation = theObject->GetPresentation();
  if (!aPresentation.IsNull())
  {
    TDF_Label aLPres = Label().FindChild (ChildLab_Presentation);
    TNaming_Builder aBuilder (aLPres);
    aBuilder.Generated (aPresentation);
  }
}

// =======================================================================
// function : ID
// purpose  :
// =======================================================================
const Standard_GUID&
XCAFDoc_Note::ID() const
{
  return GetID();
}

// =======================================================================
// function : NewEmpty
// purpose  :
// =======================================================================
Handle(TDF_Attribute)
XCAFDoc_Note::NewEmpty() const
{
  return new XCAFDoc_Note();
}

// =======================================================================
// function : Restore
// purpose  :
// =======================================================================
void
XCAFDoc_Note::Restore(const Handle(TDF_Attribute)& theAttr)
{
  myUserName = Handle(XCAFDoc_Note)::DownCast(theAttr)->myUserName;
  myTimeStamp = Handle(XCAFDoc_Note)::DownCast(theAttr)->myTimeStamp;
}

// =======================================================================
// function : Paste
// purpose  :
// =======================================================================
void
XCAFDoc_Note::Paste(const Handle(TDF_Attribute)&       theAttrInto,
                    const Handle(TDF_RelocationTable)& /*theRT*/) const
{
  Handle(XCAFDoc_Note)::DownCast(theAttrInto)->Set(myUserName, myTimeStamp);
}

// =======================================================================
// function : Dump
// purpose  :
// =======================================================================
Standard_OStream&
XCAFDoc_Note::Dump(Standard_OStream& theOS) const
{
  TDF_Attribute::Dump(theOS);
  theOS 
    << "Note : " 
    << (myUserName.IsEmpty() ? myUserName : "<anonymous>")
    << " on "
    << (myTimeStamp.IsEmpty() ? myTimeStamp : "<unknown>")
    ;
  return theOS;
}
