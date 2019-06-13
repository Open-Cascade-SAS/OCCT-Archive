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

#ifndef _XCAFDoc_NoteComment_HeaderFile
#define _XCAFDoc_NoteComment_HeaderFile

#include <Standard_Handle.hxx>
#include <Standard_Transient.hxx>
#include <TDF_Label.hxx>

class TDataStd_Comment;

//! Comment note proxy.
//! Handles a textual comment of the note.
class XCAFDoc_NoteComment : public Standard_Transient
{
public:

  DEFINE_STANDARD_RTTIEXT(XCAFDoc_NoteComment, Standard_Transient)

  //! Finds a reference attribute on the given label and returns a proxy instance if it is found
  Standard_EXPORT static Handle(XCAFDoc_NoteComment) Get(const TDF_Label& theLabel);

  //! Create (if not exist) a comment on the given note label.
  //! \param [in] theLabel   - note label.
  //! \param [in] theComment - comment text.
  Standard_EXPORT static Handle(XCAFDoc_NoteComment) Set(const TDF_Label&                  theLabel,
                                                         const TCollection_ExtendedString& theComment);

  //! Sets the comment text.
  Standard_EXPORT void Set(const TCollection_ExtendedString& theComment);

  //! Returns the comment text.
  Standard_EXPORT const TCollection_ExtendedString& Get() const;

  //! Returns label
  Standard_EXPORT TDF_Label Label() const;

private:

  XCAFDoc_NoteComment(const Handle(TDataStd_Comment)& theComment);

  Handle(TDataStd_Comment) myComment; ///< Comment attribute.

};

DEFINE_STANDARD_HANDLE(XCAFDoc_NoteComment, Standard_Transient)

#endif // _XCAFDoc_NoteComment_HeaderFile
