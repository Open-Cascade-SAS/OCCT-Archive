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

#ifndef _XCAFDoc_AnimationTool_HeaderFile
#define _XCAFDoc_AnimationTool_HeaderFile

#include <TDF_LabelSequence.hxx>
#include <TColStd_HArray1OfByte.hxx>
#include <OSD_File.hxx>

class XCAFDoc_AnimationTool;
DEFINE_STANDARD_HANDLE(XCAFDoc_AnimationToolTool, TDF_Attribute)

//! Provide tool for management of Animations section of document.
//! Provide tool to store, retrieve, remove and modify animations.
//! Each animation has a name and binary data of screenshot and animation.
class XCAFDoc_AnimationTool : public TDF_Attribute
{

public:

  
  Standard_EXPORT XCAFDoc_AnimationTool();
  
  //! Creates (if not exist) Animation tool.
  Standard_EXPORT static Handle(XCAFDoc_AnimationTool) Set(const TDF_Label& theLabel);
  
  Standard_EXPORT static const Standard_GUID& GetID();
  
  //! returns the label under which Animations are stored
  Standard_EXPORT TDF_Label BaseLabel() const;
  
  //! Returns True if label belongs to a Animations table and
  //! is a animation definition
  Standard_EXPORT Standard_Boolean IsAnimation (const TDF_Label& theLabel) const;
  
  //! Returns animation defined by label theLabel
  //! Returns False if the label is not in Animations table
  Standard_EXPORT Standard_Boolean GetAnimation(const TDF_Label& theLabel, Handle(TCollection_HAsciiString)& theName, Handle(TColStd_HArray1OfByte)& theImage, Handle(TColStd_HArray1OfByte)& theAnimation) const;

  //! Adds an animation definition to a Animations table
  //! Returns created label 
  Standard_EXPORT TDF_Label AddAnimation(const Handle(TCollection_HAsciiString)& theName, const Handle(TColStd_HArray1OfByte)& theImage, const Handle(TColStd_HArray1OfByte)& theAnimation);
 
  //! Adds an animation definition to a Animations table
  //! Returns created label 
  Standard_EXPORT TDF_Label AddAnimation(const Handle(TCollection_HAsciiString)& theName, OSD_File& theImageFile, OSD_File& theAnimationFile);
  
  //! Removes animation from the Animations table
  Standard_EXPORT Standard_Boolean RemoveAnimation(const TDF_Label& theLabel) const;
  
  //! Returns a sequence of animations currently stored
  //! in the Animations table
  Standard_EXPORT void GetAnimationLabels(TDF_LabelSequence& Labels) const;
  
  //! Rename animation
  Standard_EXPORT void SetName(const TDF_Label& theLabelL, const Handle(TCollection_HAsciiString)& theName) const;
  
  Standard_EXPORT const Standard_GUID& ID() const Standard_OVERRIDE;
  
  Standard_EXPORT void Restore (const Handle(TDF_Attribute)& with) Standard_OVERRIDE;
  
  Standard_EXPORT Handle(TDF_Attribute) NewEmpty() const Standard_OVERRIDE;
  
  Standard_EXPORT void Paste (const Handle(TDF_Attribute)& into, const Handle(TDF_RelocationTable)& RT) const Standard_OVERRIDE;




  DEFINE_STANDARD_RTTIEXT(XCAFDoc_AnimationTool, TDF_Attribute)

};
#endif // _XCAFDoc_ClippingPlaneTool_HeaderFile
