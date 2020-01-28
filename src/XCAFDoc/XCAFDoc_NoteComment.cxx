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

#include <XCAFDoc_NoteComment.hxx>

#include <Standard_GUID.hxx>
#include <TDataStd_Comment.hxx>
#include <XCAFDoc_Note.hxx>

IMPLEMENT_STANDARD_RTTIEXT(XCAFDoc_NoteComment, Standard_Transient)

// =======================================================================
// function : Get
// purpose  :
// =======================================================================
Handle(XCAFDoc_NoteComment)
XCAFDoc_NoteComment::Get(const TDF_Label& theLabel)
{
  if (!XCAFDoc_Note::IsMine(theLabel))
    return NULL;

  Handle(TDataStd_Comment) aComment;
  if (!theLabel.FindAttribute(TDataStd_Comment::GetID(), aComment))
    return NULL;

  return new XCAFDoc_NoteComment(aComment);
}

// =======================================================================
// function : Set
// purpose  :
// =======================================================================
Handle(XCAFDoc_NoteComment)
XCAFDoc_NoteComment::Set(const TDF_Label&                  theLabel,
                         const TCollection_ExtendedString& theComment)
{
  return XCAFDoc_Note::IsMine(theLabel) 
    ? new XCAFDoc_NoteComment(TDataStd_Comment::Set(theLabel, theComment))
    : NULL
    ;
}

// =======================================================================
// function : XCAFDoc_NoteComment
// purpose  :
// =======================================================================
XCAFDoc_NoteComment::XCAFDoc_NoteComment(const Handle(TDataStd_Comment)& theComment)
  : myComment(theComment)
{
}

// =======================================================================
// function : Set
// purpose  :
// =======================================================================
void
XCAFDoc_NoteComment::Set(const TCollection_ExtendedString& theComment)
{
  if (!myComment.IsNull())
    myComment->Set(theComment);
}

// =======================================================================
// function : Get
// purpose  :
// =======================================================================
const TCollection_ExtendedString& 
XCAFDoc_NoteComment::Get() const
{
  return myComment->Get();
}

// =======================================================================
// function : Label
// purpose  :
// =======================================================================
TDF_Label
XCAFDoc_NoteComment::Label() const
{
  return !myComment.IsNull() ? myComment->Label() : TDF_Label();
}
