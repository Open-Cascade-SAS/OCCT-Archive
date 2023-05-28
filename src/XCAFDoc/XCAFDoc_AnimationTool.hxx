// Copyright (c) 2023 OPEN CASCADE SAS
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

#ifndef _XCAFDoc_AnimationTool_HeaderFile
#define _XCAFDoc_AnimationTool_HeaderFile

#include <TDF_LabelSequence.hxx>
#include <TDataStd_GenericEmpty.hxx>

//! A tool to store shapes in an XDE
//! document in the form of assembly structure, and to maintain this structure.
//! Attribute containing Shapes section of DECAF document.
//! Provide tools for management of Shapes section.
//! The API provided by this class allows to work with this
//! structure regardless of its low-level implementation.
//! All the shapes are stored on child labels of a main label which is
//! XCAFDoc_DocumentTool::LabelShapes(). The label for assembly also has
//! sub-labels, each of which represents the instance of
//! another shape in that assembly (component). Such sub-label
//! stores reference to the label of the original shape in the form
//! of TDataStd_TreeNode with GUID XCAFDoc::ShapeRefGUID(), and its
//! location encapsulated into the NamedShape.
//! For correct work with an XDE document, it is necessary to use
//! methods for analysis and methods for working with shapes.
class XCAFDoc_AnimationTool : public TDataStd_GenericEmpty
{

public:

  Standard_EXPORT static const Standard_GUID& GetID();

  //! Create (if not exist) ShapeTool from XCAFDoc on <L>.
  Standard_EXPORT static Handle(XCAFDoc_AnimationTool) Set(const TDF_Label& theLabel);

  //! Creates an empty tool
  //! Creates a tool to work with a document <Doc>
  //! Attaches to label XCAFDoc::LabelShapes()
  Standard_EXPORT XCAFDoc_AnimationTool();

  //! returns the label under which shapes are stored
  Standard_EXPORT TDF_Label BaseLabel() const;

  //!
  Standard_EXPORT bool IsAnimation(const TDF_Label& theLabel) const;

  //!
  Standard_EXPORT void SetAnimation(const TDF_Label& theShLabel,
                                    const TDF_Label& theAnimLabel) const;

  //!
  Standard_EXPORT bool GetGlobalFPS(double& theFPS) const;

  //!
  Standard_EXPORT void SetGlobalFPS(const double theFPS) const;

  //! Returns a sequence of Animation labels currently stored
  //! in the Animation table.
  Standard_EXPORT void GetAnimationLabels(TDF_LabelSequence& theLabels) const;

  //! Returns Animation label defined for shape.
  Standard_EXPORT bool GetRefAnimationLabel(const TDF_Label& theShLabel,
                                            TDF_Label& theAnimLabel) const;

  //! Returns Animation label defined for shape.
  Standard_EXPORT bool GetRefShapeLabel(const TDF_Label& theAnimLabel,
                                        TDF_Label& theShLabel) const;

  //! Adds a animation definition to the Animation table and returns its label.
  Standard_EXPORT TDF_Label AddAnimation() const;

  //! Returns true if the given Animation is marked as locked.
  Standard_EXPORT bool IsLocked(const TDF_Label& theAnimLabel) const;

  //! Mark the given Animation as locked.
  Standard_EXPORT void Lock(const TDF_Label& theAnimLabel) const;

  //! Unlock the given Animation.
  Standard_EXPORT void Unlock(const TDF_Label& theAnimLabel) const;

  Standard_EXPORT Standard_OStream& Dump(Standard_OStream& theOStream, const bool theDepth) const;

  Standard_EXPORT virtual Standard_OStream& Dump(Standard_OStream& theOStream) const Standard_OVERRIDE;

  Standard_EXPORT const Standard_GUID& ID() const Standard_OVERRIDE;

  //! Dumps the content of me into the stream
  Standard_EXPORT virtual void DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth = -1) const Standard_OVERRIDE;

  DEFINE_DERIVED_ATTRIBUTE(XCAFDoc_AnimationTool, TDataStd_GenericEmpty)

private:

};

#endif // _XCAFDoc_AnimationTool_HeaderFile
